/*
    Copyright (C) 2017 -  B. J. Hill

    This file is part of open62541 C++ classes. open62541 C++ classes are free software: you can
    redistribute it and/or modify it under the terms of the Mozilla Public
    License v2.0 as stated in the LICENSE file provided with open62541.

    open62541 C++ classes are distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
    A PARTICULAR PURPOSE.
*/
#ifndef OPEN62541CLIENT_H
#define OPEN62541CLIENT_H
#include <open62541cpp/open62541objects.h>
#include <open62541cpp/clientsubscription.h>

/*
    OPC nodes are just data objects they do not need to be in a property tree
    nodes can be refered to by name or number (or GUID) which is  hash index to the item in the server
*/

namespace Open62541 {

// Only really for receiving lists  not safe to copy
class UA_EXPORT ApplicationDescriptionList : public std::vector<UA_ApplicationDescription*>
{
public:
    ApplicationDescriptionList() {}
    ~ApplicationDescriptionList()
    {
        for (auto i : *this) {
            if (i) {
                UA_ApplicationDescription_delete(i);  // delete the item
            }
        }
    }
};

// dictionary of subscriptions associated with a Client
typedef std::shared_ptr<ClientSubscription> ClientSubscriptionRef;
//
typedef std::map<UA_UInt32, ClientSubscriptionRef> ClientSubscriptionMap;
//
/*!
    \brief The Client class
    This class wraps the corresponding C functions. Refer to the C documentation for a full explanation.
    The main thing to watch for is Node ID objects are passed by reference. There are stock Node Id objects including
   NodeId::Null Pass NodeId::Null where a NULL UA_NodeId pointer is expected. If a NodeId is being passed to receive a
   value use the notNull() method to mark it as a receiver of a new node id. Most functions return true if the lastError
   is UA_STATUSCODE_GOOD.
*/

class Client
{

public:
    enum ConnectionType { NONE, CONNECTION, ASYNC, SECURE, SECUREASYNC };

    /*!
     * \brief The Timer class - used for timed events
     */
    class Timer
    {
        Client* _client = nullptr;
        UA_UInt64 _id   = 0;
        bool _oneShot   = false;
        std::function<void(Timer&)> _handler;

    public:
        Timer() {}
        Timer(Client* c, UA_UInt64 i, bool os, std::function<void(Timer&)> func)
            : _client(c)
            , _id(i)
            , _oneShot(os)
            , _handler(func)
        {
        }
        virtual ~Timer() { UA_Client_removeCallback(_client->client(), _id); }
        virtual void handle()
        {
            if (_handler)
                _handler(*this);
        }
        Client* client() const { return _client; }
        UA_UInt64 id() const { return _id; }
        void setId(UA_UInt64 i) { _id = i; }
        bool oneShot() const { return _oneShot; }
    };

    typedef std::unique_ptr<Timer> TimerPtr;

private:
    UA_Client* _client = nullptr;
    ReadWriteMutex _mutex;
    //
    ClientSubscriptionMap _subscriptions;
    //
    // Track states to trigger notifications of changes
    UA_SecureChannelState _lastSecureChannelState = UA_SECURECHANNELSTATE_CLOSED;
    UA_SessionState _lastSessionState             = UA_SESSIONSTATE_CLOSED;
    //
    ConnectionType _connectionType = ConnectionType::NONE;

    std::map<UA_UInt64, TimerPtr> _timerMap;  // one map per client

    // status
    UA_SecureChannelState _channelState = UA_SECURECHANNELSTATE_CLOSED;
    UA_SessionState _sessionState       = UA_SESSIONSTATE_CLOSED;
    UA_StatusCode _connectStatus        = UA_STATUSCODE_GOOD;

protected:
    UA_StatusCode _lastError = 0;

private:
    // Call Backs
    static void stateCallback(UA_Client* client,
                              UA_SecureChannelState channelState,
                              UA_SessionState sessionState,
                              UA_StatusCode connectStatus);
    /*!
        \brief asyncConnectCallback
        \param client
        \param userdata
        \param requestId
        \param response
    */
    static void asyncConnectCallback(UA_Client* client, void* userdata, UA_UInt32 requestId, void* response)
    {
        Client* p = (Client*)(UA_Client_getContext(client));
        if (p) {
            p->asyncConnectService(requestId, userdata, response);
        }
    }

    /*!
     * \brief clientCallback
     * \param client
     * \param data
     */
    static void clientCallback(UA_Client* /*client*/, void* data)
    {
        // timer callback
        if (data) {
            Timer* t = static_cast<Timer*>(data);
            if (t) {
                t->handle();
                if (t->oneShot()) {
                    // Potential risk of the client disappearing
                    t->client()->_timerMap.erase(t->id());
                }
            }
        }
    }

public:
    // must connect to have a valid client
    Client()
        : _client(nullptr)
    {
    }

    /*!
        \brief ~Open62541Client
    */
    virtual ~Client()
    {
        if (_client) {
            _timerMap.clear();
            disconnect();
            UA_Client_delete(_client);
        }
    }

    /*!
     * \brief connectionType
     * \return connection type
     */
    ConnectionType connectionType() const { return _connectionType; }
    /*!
     * \brief setConnectionType
     * \param c
     */
    void setConnectionType(ConnectionType c) { _connectionType = c; }

    /*!
     * \brief runIterate
     * \param interval
     * \return
     */
    bool runIterate(uint32_t interval = 100)
    {
        if (_client && (_connectStatus == UA_STATUSCODE_GOOD)) {
            _lastError = UA_Client_run_iterate(_client, interval);
            return lastOK();
        }
        return false;
    }

    /*!
     * \brief run
     * \return true on success
     */
    bool run()
    {
        while (runIterate() && process())
            ;  // runs until disconnect
        return true;
    }
    /*!
     * \brief initialise
     */
    void initialise()
    {
        if (_client) {
            disconnect();
            UA_Client_delete(_client);
            _client = nullptr;
        }
        _client = UA_Client_new();
        if (_client) {
            UA_ClientConfig_setDefault(UA_Client_getConfig(_client));  // initalise the client structure
            UA_Client_getConfig(_client)->clientContext                  = this;
            UA_Client_getConfig(_client)->stateCallback                  = stateCallback;
            UA_Client_getConfig(_client)->subscriptionInactivityCallback = subscriptionInactivityCallback;
        }
    }
    /*!
        \brief asyncService - handles callbacks when connected async mode
        \param requestId
        \param response
    */
    virtual void asyncConnectService(UA_UInt32 /*requestId*/, void* /*userData*/, void* /*response*/) {}
    /*!
        \brief getContext
        \return
    */
    void* getContext() { return UA_Client_getContext(client()); }

    /*!
        \brief subscriptionInactivityCallback
        \param client
        \param subscriptionId
        \param subContext
    */
    static void subscriptionInactivityCallback(UA_Client* client, UA_UInt32 subscriptionId, void* subContext);
    /*!
        \brief subscriptionInactivity
        \param subscriptionId
        \param subContext
    */
    virtual void subscriptionInactivity(UA_UInt32 /*subscriptionId*/, void* /*subContext*/) {}

    /*!
        \brief subscriptions
        \return map of subscriptions
    */
    ClientSubscriptionMap& subscriptions() { return _subscriptions; }
    /*!
        \brief addSubscription
        \param newId receives Id of created subscription
        \return true on success
    */
    bool addSubscription(UA_UInt32& newId, CreateSubscriptionRequest* settings = nullptr)
    {
        //
        ClientSubscriptionRef c(new ClientSubscription(*this));
        //
        if (settings) {
            c->settings() = *settings;  // assign settings across
        }
        //
        if (c->create()) {
            newId                  = c->id();
            subscriptions()[newId] = c;
            return true;
        }
        //
        return false;
    }

    /*!
        \brief removeSubscription
        \param Id
        \return true on success
    */
    bool removeSubscription(UA_UInt32 Id)
    {
        subscriptions().erase(Id);  // remove from dictionary implicit delete
        return true;
    }

    /*!
        \brief subscription
        \param Id
        \return pointer to subscription object or null
    */
    ClientSubscription* subscription(UA_UInt32 Id)
    {
        if (subscriptions().find(Id) != subscriptions().end()) {
            ClientSubscriptionRef& c = subscriptions()[Id];
            return c.get();
        }
        return nullptr;
    }

    //
    // Connection state handlers
    //
    virtual void SecureChannelStateClosed()
    {
        subscriptions().clear();
        _timerMap.clear();
        OPEN62541_TRC
    }
    virtual void SecureChannelStateHelSent() { OPEN62541_TRC }
    virtual void SecureChannelStateHelReceived() { OPEN62541_TRC }
    virtual void SecureChannelStateAckSent() { OPEN62541_TRC }
    virtual void SecureChannelStateAckReceived() { OPEN62541_TRC }
    virtual void SecureChannelStateOpenSent() { OPEN62541_TRC }
    virtual void SecureChannelStateOpen() { OPEN62541_TRC }
    virtual void SecureChannelStateClosing()
    {
        subscriptions().clear();
        _timerMap.clear();
        OPEN62541_TRC
    }
    //
    // Session handlers
    //
    virtual void SessionStateClosed()
    {
        subscriptions().clear();
        _timerMap.clear();
        OPEN62541_TRC
    }
    virtual void SessionStateCreateRequested() { OPEN62541_TRC }
    virtual void SessionStateCreated() { OPEN62541_TRC }
    virtual void SessionStateActivateRequested() { OPEN62541_TRC }
    virtual void SessionStateActivated() { OPEN62541_TRC }
    virtual void SessionStateClosing()
    {
        subscriptions().clear();
        OPEN62541_TRC
    }

    // connection has had a fault
    virtual void connectFail() { OPEN62541_TRC }

    /*!
        \brief stateChange
        \param clientState
    */
    virtual void stateChange(UA_SecureChannelState channelState,
                             UA_SessionState sessionState,
                             UA_StatusCode connectStatus);

    /*!
        \brief getEndpoints
        Retrive end points
        \param serverUrl
        \param list
        \return true on success
    */
    bool getEndpoints(const std::string& serverUrl, EndpointDescriptionArray& list)
    {
        if (!_client)
            return false;
        WriteLock l(_mutex);
        size_t endpointDescriptionsSize              = 0;
        UA_EndpointDescription* endpointDescriptions = nullptr;
        _lastError =
            UA_Client_getEndpoints(_client, serverUrl.c_str(), &endpointDescriptionsSize, &endpointDescriptions);
        if (lastOK()) {
            // copy list so it is managed by the caller
            list.setList(endpointDescriptionsSize, endpointDescriptions);
        }
        return lastOK();
    }

    /*!
        \brief findServers
        \param serverUrl
        \param serverUris
        \param localeIds
        \param registeredServers
        \return true on success
    */
    bool findServers(const std::string& serverUrl,
                     StringArray& serverUris,
                     StringArray& localeIds,
                     ApplicationDescriptionArray& registeredServers)
    {
        if (!_client)
            return false;
        WriteLock l(_mutex);
        _lastError = UA_Client_findServers(_client,
                                           serverUrl.c_str(),
                                           serverUris.length(),
                                           serverUris.data(),
                                           localeIds.length(),
                                           localeIds.data(),
                                           registeredServers.lengthRef(),
                                           registeredServers.dataRef());
        UAPRINTLASTERROR(_lastError)
        return lastOK();
    }

    /*!
        \brief findServersOnNetwork
        \param serverUrl
        \param startingRecordId
        \param maxRecordsToReturn
        \param serverCapabilityFilter
        \param serverOnNetwork
        \return true on success
    */
    bool findServersOnNetwork(const std::string& serverUrl,
                              unsigned startingRecordId,
                              unsigned maxRecordsToReturn,
                              StringArray& serverCapabilityFilter,
                              ServerOnNetworkArray& serverOnNetwork)
    {
        if (!_client)
            return false;
        WriteLock l(_mutex);
        _lastError = UA_Client_findServersOnNetwork(_client,
                                                    serverUrl.c_str(),
                                                    startingRecordId,
                                                    maxRecordsToReturn,
                                                    serverCapabilityFilter.length(),
                                                    serverCapabilityFilter.data(),
                                                    serverOnNetwork.lengthRef(),
                                                    serverOnNetwork.dataRef());
        return lastOK();
    }
    /*!
        \brief readAttribute
        \param nodeId
        \param attributeId
        \param out
        \param outDataType
        \return true on success
    */
    bool readAttribute(const UA_NodeId* nodeId, UA_AttributeId attributeId, void* out, const UA_DataType* outDataType)
    {
        if (!_client)
            return false;
        WriteLock l(_mutex);
        _lastError = __UA_Client_readAttribute(_client, nodeId, attributeId, out, outDataType);
        return lastOK();
    }

    /*!
        \brief writeAttribute
        \param nodeId
        \param attributeId
        \param in
        \param inDataType
        \return true on success
    */
    bool writeAttribute(const UA_NodeId* nodeId,
                        UA_AttributeId attributeId,
                        const void* in,
                        const UA_DataType* inDataType)
    {
        if (!_client)
            return false;
        WriteLock l(_mutex);
        _lastError = __UA_Client_writeAttribute(_client, nodeId, attributeId, in, inDataType);
        return lastOK();
    }

    /*!
        \brief mutex
        \return  client read/write mutex
    */
    ReadWriteMutex& mutex() { return _mutex; }
    /*!
        \brief getState
        \return connection state
    */
    UA_StatusCode getState(UA_SecureChannelState& channelState, UA_SessionState& sessionState)
    {
        ReadLock l(_mutex);
        if (_client) {
            UA_StatusCode c;
            UA_Client_getState(_client, &channelState, &sessionState, &c);
            return c;
        }
        throw std::runtime_error("Null client");
        return UA_STATUSCODE_BADCONNECTIONCLOSED;
    }

    /*!
        \brief reset - removed from 1.1.1
    */

    /*!
        \brief client
        \return underlying client object
    */
    UA_Client* client()
    {
        ReadLock l(_mutex);
        return _client;
    }
    //
    /*!
        \brief config
        \return client configuration
    */
    UA_ClientConfig& config() { return *UA_Client_getConfig(_client); }
    /*!
        \brief lastError
        \return last error set
    */

    UA_StatusCode lastError() { return _lastError; }
    //
    // Connect and Disconnect
    //
    /*!
        \brief connect
        \param endpointUrl
        \return true on success
    */
    bool connect(const std::string& endpointUrl)
    {
        initialise();
        WriteLock l(_mutex);
        if (!_client)
            throw std::runtime_error("Null client");
        _lastError = UA_Client_connect(_client, endpointUrl.c_str());
        if (lastOK()) {
            _connectionType = ConnectionType::CONNECTION;
        }
        else {
            _connectionType = ConnectionType::NONE;
        }
        return lastOK();
    }

    /*!  Connect to the selected server with the given username and password

        @param client to use
        @param endpointURL to connect (for example "opc.tcp://localhost:16664")
        @param username
        @param password
        @return Indicates whether the operation succeeded or returns an error code */
    bool connectUsername(const std::string& endpoint, const std::string& username, const std::string& password)
    {
        initialise();
        WriteLock l(_mutex);
        if (!_client)
            throw std::runtime_error("Null client");
        _lastError = UA_Client_connectUsername(_client, endpoint.c_str(), username.c_str(), password.c_str());
        if (lastOK()) {
            _connectionType = ConnectionType::CONNECTION;
        }
        else {
            _connectionType = ConnectionType::NONE;
        }
        return lastOK();
    }
    /*!
        \brief connectAsync
        \param endpoint
        \return Indicates whether the operation succeeded or returns an error code
    */
    bool connectAsync(const std::string& endpoint)
    {
        initialise();
        WriteLock l(_mutex);
        if (!_client)
            throw std::runtime_error("Null client");
        _lastError = UA_Client_connectAsync(_client, endpoint.c_str());
        return lastOK();
        if (lastOK()) {
            _connectionType = ConnectionType::ASYNC;
        }
        else {
            _connectionType = ConnectionType::NONE;
        }
    }

    /*!
     * \brief connectSecureChannel
     * \param endpoint
     * \return Indicates whether the operation succeeded or returns an error code
     */
    bool connectSecureChannel(const std::string& endpoint)
    {
        initialise();
        WriteLock l(_mutex);
        if (!_client)
            throw std::runtime_error("Null client");
        _lastError = UA_Client_connectSecureChannel(_client, endpoint.c_str());
        if (lastOK()) {
            _connectionType = ConnectionType::SECURE;
        }
        else {
            _connectionType = ConnectionType::NONE;
        }

        return lastOK();
    }

    /*!
     * \brief connectSecureChannelAsync
     * \param endpoint
     * \return Indicates whether the operation succeeded or returns an error code
     */
    bool connectSecureChannelAsync(const std::string& endpoint)
    {
        initialise();
        WriteLock l(_mutex);
        if (!_client)
            throw std::runtime_error("Null client");
        _lastError = UA_Client_connectSecureChannelAsync(_client, endpoint.c_str());
        if (lastOK()) {
            _connectionType = ConnectionType::SECUREASYNC;
        }
        else {
            _connectionType = ConnectionType::NONE;
        }

        return lastOK();
    }

    /*!
        \brief disconnect
        \return
    */
    bool disconnect()
    {
        WriteLock l(_mutex);
        if (!_client)
            throw std::runtime_error("Null client");
        // close subscriptions
        subscriptions().clear();
        _timerMap.clear();  // remove timer objects
        _lastError      = UA_Client_disconnect(_client);
        _connectionType = ConnectionType::NONE;
        return lastOK();
    }
    /*!
        \brief disconnectAsync
        \return true on success
    */
    bool disconnectAsync()
    {
        WriteLock l(_mutex);
        if (!_client)
            throw std::runtime_error("Null client");
        _timerMap.clear();  // remove timer objects
        _lastError      = UA_Client_disconnectAsync(_client);
        _connectionType = ConnectionType::NONE;
        return lastOK();
    }

    bool disconnectSecureChannel()
    {
        WriteLock l(_mutex);
        if (!_client)
            throw std::runtime_error("Null client");
        _timerMap.clear();  // remove timer objects
        if ((_connectionType == ConnectionType::SECURE) || (_connectionType == ConnectionType::SECUREASYNC)) {
            _lastError      = UA_Client_disconnectSecureChannel(_client);
            _connectionType = ConnectionType::NONE;
        }
        else {
            throw std::runtime_error("Not a secure connection");
        }
        return lastOK();
    }

    /*!
        \brief manuallyRenewSecureChannel
        \return
    */
    bool manuallyRenewSecureChannel() { return false; }

    /*!  Gets a list of endpoints of a server

        @param client to use. Must be connected to the same endpoint given in
              serverUrl or otherwise in disconnected state.
        @param serverUrl url to connect (for example "opc.tcp://localhost:16664")
        @param endpointDescriptionsSize size of the array of endpoint descriptions
        @param endpointDescriptions array of endpoint descriptions that is allocated
              by the function (you need to free manually)
        @return Indicates whether the operation succeeded or returns an error code */

    //
    // only use for getting string names of end points
    UA_StatusCode getEndpoints(const std::string& serverUrl, std::vector<std::string>& list);

    /*!  Get the namespace-index of a namespace-URI

        @param client The UA_Client struct for this connection
        @param namespaceUri The interested namespace URI
        @param namespaceIndex The namespace index of the URI. The value is unchanged
              in case of an error
        @return Indicates whether the operation succeeded or returns an error code */
    int namespaceGetIndex(const std::string& namespaceUri)
    {
        WriteLock l(_mutex);
        if (!_client)
            throw std::runtime_error("Null client");
        int namespaceIndex = 0;
        UA_String s        = toUA_String(namespaceUri);
        if (UA_Client_NamespaceGetIndex(_client, &s, (UA_UInt16*)(&namespaceIndex)) == UA_STATUSCODE_GOOD) {
            return namespaceIndex;
        }
        return -1;  // value
    }

    /*!
        \brief browseName
        \param nodeId
        \return true on success
    */
    bool browseName(NodeId& nodeId, std::string& s, int& ns)
    {
        WriteLock l(_mutex);
        if (!_client)
            throw std::runtime_error("Null client");
        QualifiedName outBrowseName;
        if ((_lastError = UA_Client_readBrowseNameAttribute(_client, nodeId, outBrowseName)) == UA_STATUSCODE_GOOD) {
            s  = toString(outBrowseName.get().name);
            ns = outBrowseName.get().namespaceIndex;
        }
        return _lastError == UA_STATUSCODE_GOOD;
    }

    /*!
        \brief setBrowseName
        \param nodeId
        \param nameSpaceIndex
        \param name
    */
    void setBrowseName(NodeId& nodeId, int nameSpaceIndex, const std::string& name)
    {
        WriteLock l(_mutex);
        if (!_client)
            throw std::runtime_error("Null client");
        QualifiedName newBrowseName(nameSpaceIndex, name);
        UA_Client_writeBrowseNameAttribute(_client, nodeId, newBrowseName);
    }

    /*!
        \brief browseTree
        \param nodeId
        \param node
        \return true on success
    */
    bool browseTree(UA_NodeId& nodeId, Open62541::UANode* node);

    /*!
        \brief browseTree
        \param nodeId
        \return true on success
    */
    bool browseTree(NodeId& nodeId, UANodeTree& tree);
    /*!
        \brief browseTree
        \param nodeId
        \param tree
        \return
    */
    bool browseTree(NodeId& nodeId, UANode* tree);
    /*!
        \brief browseTree
        \param nodeId
        \param tree
        \return
    */
    bool browseTree(NodeId& nodeId, NodeIdMap& m);  // browse and create a map of string ids to node ids
    /*!
        \brief browseChildren
        \param nodeId
        \param m
        \return  true on success
    */
    bool browseChildren(UA_NodeId& nodeId, NodeIdMap& m);

    /*!
        \brief NodeIdFromPath get the node id from the path of browse names in the given namespace. Tests for node
       existance \param path \param nodeId \return  true on success
    */
    bool nodeIdFromPath(const NodeId &start, Path& path, NodeId& nodeId);

    /*!
        \brief createPath
        \param start
        \param path
        \param nameSpaceIndex
        \param nodeId
        \return  true on success
    */
    bool createFolderPath(const NodeId& start, Path& path, int nameSpaceIndex, NodeId& nodeId);

    /*!
        \brief getChild
        \param nameSpaceIndex
        \param childName
        \return
    */
    bool getChild(const NodeId& start, const std::string& childName, NodeId& ret);

    /*!
        \brief addFolder
        \param parent
        \param nameSpaceIndex
        \param childName
        \return  true on success
    */
    bool addFolder(NodeId& parent,
                   const std::string& childName,
                   NodeId& nodeId,
                   NodeId& newNode    = NodeId::Null,
                   int nameSpaceIndex = 0);

    /*!
        \brief addVariable
        \param parent
        \param nameSpaceIndex
        \param childName
        \return  true on success
    */
    bool addVariable(NodeId& parent,
                     const std::string& childName,
                     Variant& value,
                     NodeId& nodeId,
                     NodeId& newNode    = NodeId::Null,
                     int nameSpaceIndex = 0);

    /*!
        \brief setVariable
        \param nodeId
        \param value
        \return  true on success
    */
    bool setVariable(NodeId& nodeId, Variant& value)
    {
        if (!_client)
            return false;
        _lastError = UA_Client_writeValueAttribute(_client, nodeId, value);
        return lastOK();
    }

    // Attribute access generated from the docs
    /*!
        \brief readNodeIdAttribute
        \param nodeId
        \param outNodeId
        \return  true on success
    */
    bool readNodeIdAttribute(NodeId& nodeId, UA_NodeId& outNodeId)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_NODEID, &outNodeId, &UA_TYPES[UA_TYPES_NODEID]);
    }
    /*!
        \brief readNodeClassAttribute
        \param nodeId
        \param outNodeClass
        \return  true on success
    */
    bool readNodeClassAttribute(NodeId& nodeId, UA_NodeClass& outNodeClass)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_NODECLASS, &outNodeClass, &UA_TYPES[UA_TYPES_NODECLASS]);
    }
    /*!
        \brief readBrowseNameAttribute
        \param nodeId
        \param outBrowseName
        \return  true on success
    */
    bool readBrowseNameAttribute(NodeId& nodeId, QualifiedName& outBrowseName)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_BROWSENAME, outBrowseName, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
    }
    /*!
        \brief readDisplayNameAttribute
        \param nodeId
        \param outDisplayName
        \return  true on success
    */
    bool readDisplayNameAttribute(NodeId& nodeId, LocalizedText& outDisplayName)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_DISPLAYNAME, outDisplayName, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
    }
    /*!
        \brief readDescriptionAttribute
        \param nodeId
        \param outDescription
        \return  true on success
    */
    bool readDescriptionAttribute(NodeId& nodeId, LocalizedText& outDescription)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_DESCRIPTION, outDescription, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
        return lastOK();
    }
    /*!
        \brief readWriteMaskAttribute
        \param nodeId
        \param outWriteMask
        \return  true on success
    */
    bool readWriteMaskAttribute(NodeId& nodeId, UA_UInt32& outWriteMask)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_WRITEMASK, &outWriteMask, &UA_TYPES[UA_TYPES_UINT32]);
    }
    /*!
        \brief readUserWriteMaskAttribute
        \param nodeId
        \param outUserWriteMask
        \return  true on success
    */
    bool readUserWriteMaskAttribute(NodeId& nodeId, UA_UInt32& outUserWriteMask)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_USERWRITEMASK, &outUserWriteMask, &UA_TYPES[UA_TYPES_UINT32]);
    }
    /*!
        \brief readIsAbstractAttribute
        \param nodeId
        \param outIsAbstract
        \return  true on success
    */
    bool readIsAbstractAttribute(NodeId& nodeId, UA_Boolean& outIsAbstract)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_ISABSTRACT, &outIsAbstract, &UA_TYPES[UA_TYPES_BOOLEAN]);
    }
    /*!
        \brief readSymmetricAttribute
        \param nodeId
        \param outSymmetric
        \return  true on success
    */
    bool readSymmetricAttribute(NodeId& nodeId, UA_Boolean& outSymmetric)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_SYMMETRIC, &outSymmetric, &UA_TYPES[UA_TYPES_BOOLEAN]);
    }
    /*!
        \brief readInverseNameAttribute
        \param nodeId
        \param outInverseName
        \return  true on success
    */
    bool readInverseNameAttribute(NodeId& nodeId, LocalizedText& outInverseName)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_INVERSENAME, outInverseName, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
    }
    /*!
        \brief readContainsNoLoopsAttribute
        \param nodeId
        \param outContainsNoLoops
        \return  true on success
    */
    bool readContainsNoLoopsAttribute(NodeId& nodeId, UA_Boolean& outContainsNoLoops)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_CONTAINSNOLOOPS, &outContainsNoLoops, &UA_TYPES[UA_TYPES_BOOLEAN]);
    }
    /*!
        \brief readEventNotifierAttribute
        \param nodeId
        \param outEventNotifier
        \return  true on success
    */
    bool readEventNotifierAttribute(NodeId& nodeId, UA_Byte& outEventNotifier)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_EVENTNOTIFIER, &outEventNotifier, &UA_TYPES[UA_TYPES_BYTE]);
    }
    /*!
        \brief readValueAttribute
        \param nodeId
        \param outValue
        \return  true on success
    */
    bool readValueAttribute(NodeId& nodeId, Variant& outValue)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_VALUE, outValue, &UA_TYPES[UA_TYPES_VARIANT]);
    }
    /*!
        \brief readDataTypeAttribute
        \param nodeId
        \param outDataType
        \return  true on success
    */
    bool readDataTypeAttribute(NodeId& nodeId, UA_NodeId& outDataType)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_DATATYPE, &outDataType, &UA_TYPES[UA_TYPES_NODEID]);
    }
    /*!
        \brief readValueRankAttribute
        \param nodeId
        \param outValueRank
        \return  true on success
    */
    bool readValueRankAttribute(NodeId& nodeId, UA_Int32& outValueRank)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_VALUERANK, &outValueRank, &UA_TYPES[UA_TYPES_INT32]);
    }
    /*!
        \brief readArrayDimensionsAttribute
        \param nodeId
        \param ret
        \return true on success
    */
    bool readArrayDimensionsAttribute(NodeId& nodeId, std::vector<UA_UInt32>& ret)
    {
        if (!_client)
            return false;
        WriteLock l(_mutex);
        size_t outArrayDimensionsSize;
        UA_UInt32* outArrayDimensions = nullptr;
        _lastError =
            UA_Client_readArrayDimensionsAttribute(_client, nodeId, &outArrayDimensionsSize, &outArrayDimensions);
        if (_lastError == UA_STATUSCODE_GOOD) {
            if (outArrayDimensions) {
                for (int i = 0; i < int(outArrayDimensionsSize); i++) {
                    ret.push_back(outArrayDimensions[i]);
                }
                UA_Array_delete(outArrayDimensions, outArrayDimensionsSize, &UA_TYPES[UA_TYPES_INT32]);
            }
        }
        return lastOK();
    }
    /*!
        \brief readAccessLevelAttribute
        \param nodeId
        \param outAccessLevel
        \return  true on success
    */
    bool readAccessLevelAttribute(NodeId& nodeId, UA_Byte& outAccessLevel)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_ACCESSLEVEL, &outAccessLevel, &UA_TYPES[UA_TYPES_BYTE]);
    }
    /*!
        \brief readUserAccessLevelAttribute
        \param nodeId
        \param outUserAccessLevel
        \return  true on success
    */
    bool readUserAccessLevelAttribute(NodeId& nodeId, UA_Byte& outUserAccessLevel)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_USERACCESSLEVEL, &outUserAccessLevel, &UA_TYPES[UA_TYPES_BYTE]);
    }
    /*!
        \brief readMinimumSamplingIntervalAttribute
        \param nodeId
        \param outMinSamplingInterval
        \return  true on success
    */
    bool readMinimumSamplingIntervalAttribute(NodeId& nodeId, UA_Double& outMinSamplingInterval)
    {
        return readAttribute(nodeId,
                             UA_ATTRIBUTEID_MINIMUMSAMPLINGINTERVAL,
                             &outMinSamplingInterval,
                             &UA_TYPES[UA_TYPES_DOUBLE]);
    }
    /*!
        \brief readHistorizingAttribute
        \param nodeId
        \param outHistorizing
        \return  true on success
    */
    bool readHistorizingAttribute(NodeId& nodeId, UA_Boolean& outHistorizing)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_HISTORIZING, &outHistorizing, &UA_TYPES[UA_TYPES_BOOLEAN]);
    }
    /*!
        \brief readExecutableAttribute
        \param nodeId
        \param outExecutable
        \return  true on success
    */
    bool readExecutableAttribute(NodeId& nodeId, UA_Boolean& outExecutable)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_EXECUTABLE, &outExecutable, &UA_TYPES[UA_TYPES_BOOLEAN]);
    }
    /*!
        \brief readUserExecutableAttribute
        \param nodeId
        \param outUserExecutable
        \return  true on success
    */
    bool readUserExecutableAttribute(NodeId& nodeId, UA_Boolean& outUserExecutable)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_USEREXECUTABLE, &outUserExecutable, &UA_TYPES[UA_TYPES_BOOLEAN]);
    }

    /*!
        \brief setNodeIdAttribute
        \param nodeId
        \param newNodeId
        \return  true on success
    */
    bool setNodeIdAttribute(NodeId& nodeId, NodeId& newNodeId)
    {
        return writeAttribute(nodeId, UA_ATTRIBUTEID_NODEID, &newNodeId, &UA_TYPES[UA_TYPES_NODEID]);
    }
    /*!
        \brief setNodeClassAttribute
        \param nodeId
        \param newNodeClass
        \return  true on success
    */
    bool setNodeClassAttribute(NodeId& nodeId, UA_NodeClass& newNodeClass)
    {
        return writeAttribute(nodeId, UA_ATTRIBUTEID_NODECLASS, &newNodeClass, &UA_TYPES[UA_TYPES_NODECLASS]);
    }
    /*!
        \brief setBrowseNameAttribute
        \param nodeId
        \param newBrowseName
        \return  true on success
    */
    bool setBrowseNameAttribute(NodeId& nodeId, QualifiedName& newBrowseName)
    {
        return writeAttribute(nodeId, UA_ATTRIBUTEID_BROWSENAME, &newBrowseName, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
    }
    /*!
        \brief setDisplayNameAttribute
        \param nodeId
        \param newDisplayName
        \return  true on success
    */
    bool setDisplayNameAttribute(NodeId& nodeId, LocalizedText& newDisplayName)
    {
        return writeAttribute(nodeId, UA_ATTRIBUTEID_DISPLAYNAME, &newDisplayName, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
    }
    /*!
        \brief setDescriptionAttribute
        \param nodeId
        \param newDescription
        \return  true on success
    */
    bool setDescriptionAttribute(NodeId& nodeId, LocalizedText& newDescription)
    {
        return writeAttribute(nodeId, UA_ATTRIBUTEID_DESCRIPTION, newDescription, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
    }
    /*!
        \brief setWriteMaskAttribute
        \param nodeId
        \param newWriteMask
        \return  true on success
    */
    bool setWriteMaskAttribute(NodeId& nodeId, UA_UInt32 newWriteMask)
    {
        return writeAttribute(nodeId, UA_ATTRIBUTEID_WRITEMASK, &newWriteMask, &UA_TYPES[UA_TYPES_UINT32]);
    }
    /*!
        \brief setUserWriteMaskAttribute
        \param nodeId
        \param newUserWriteMask
        \return  true on success
    */
    bool setUserWriteMaskAttribute(NodeId& nodeId, UA_UInt32 newUserWriteMask)
    {
        return writeAttribute(nodeId, UA_ATTRIBUTEID_USERWRITEMASK, &newUserWriteMask, &UA_TYPES[UA_TYPES_UINT32]);
    }
    /*!
        \brief setIsAbstractAttribute
        \param nodeId
        \param newIsAbstract
        \return  true on success
    */
    bool setIsAbstractAttribute(NodeId& nodeId, UA_Boolean newIsAbstract)
    {
        return writeAttribute(nodeId, UA_ATTRIBUTEID_ISABSTRACT, &newIsAbstract, &UA_TYPES[UA_TYPES_BOOLEAN]);
    }
    /*!
        \brief setSymmetricAttribute
        \param nodeId
        \param newSymmetric
        \return  true on success
    */
    bool setSymmetricAttribute(NodeId& nodeId, UA_Boolean newSymmetric)
    {
        return writeAttribute(nodeId, UA_ATTRIBUTEID_SYMMETRIC, &newSymmetric, &UA_TYPES[UA_TYPES_BOOLEAN]);
    }
    /*!
        \brief setInverseNameAttribute
        \param nodeId
        \param newInverseName
        \return  true on success
    */
    bool setInverseNameAttribute(NodeId& nodeId, LocalizedText& newInverseName)
    {
        return writeAttribute(nodeId, UA_ATTRIBUTEID_INVERSENAME, &newInverseName, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
    }
    /*!
        \brief setContainsNoLoopsAttribute
        \param nodeId
        \param newContainsNoLoops
        \return  true on success
    */
    bool setContainsNoLoopsAttribute(NodeId& nodeId, UA_Boolean& newContainsNoLoops)
    {
        return writeAttribute(nodeId, UA_ATTRIBUTEID_CONTAINSNOLOOPS, &newContainsNoLoops, &UA_TYPES[UA_TYPES_BOOLEAN]);
    }
    /*!
        \brief setEventNotifierAttribute
        \param nodeId
        \param newEventNotifier
        \return  true on success
    */
    bool setEventNotifierAttribute(NodeId& nodeId, UA_Byte newEventNotifier)
    {
        return writeAttribute(nodeId, UA_ATTRIBUTEID_EVENTNOTIFIER, &newEventNotifier, &UA_TYPES[UA_TYPES_BYTE]);
    }
    /*!
        \brief setValueAttribute
        \param nodeId
        \param newValue
        \return  true on success
    */
    bool setValueAttribute(NodeId& nodeId, Variant& newValue)
    {
        return writeAttribute(nodeId, UA_ATTRIBUTEID_VALUE, newValue, &UA_TYPES[UA_TYPES_VARIANT]);
    }
    /*!
        \brief setDataTypeAttribute
        \param nodeId
        \param newDataType
        \return  true on success
    */
    bool setDataTypeAttribute(NodeId& nodeId, const UA_NodeId* newDataType)
    {
        return writeAttribute(nodeId, UA_ATTRIBUTEID_DATATYPE, newDataType, &UA_TYPES[UA_TYPES_NODEID]);
    }
    /*!
        \brief setValueRankAttribute
        \param nodeId
        \param newValueRank
        \return   true on success
    */
    bool setValueRankAttribute(NodeId& nodeId, UA_Int32 newValueRank)
    {
        return writeAttribute(nodeId, UA_ATTRIBUTEID_VALUERANK, &newValueRank, &UA_TYPES[UA_TYPES_INT32]);
    }
    /*!
        \brief setArrayDimensionsAttribute
        \param nodeId
        \param newArrayDimensions
        \return   true on success
    */
    bool setArrayDimensionsAttribute(NodeId& nodeId, std::vector<UA_UInt32>& newArrayDimensions)
    {
        UA_UInt32 v = newArrayDimensions.size();
        _lastError  = UA_Client_writeArrayDimensionsAttribute(_client, nodeId, v, newArrayDimensions.data());
        return lastOK();
    }
    /*!
        \brief setAccessLevelAttribute
        \param nodeId
        \param newAccessLevel
        \return   true on success
    */
    bool setAccessLevelAttribute(NodeId& nodeId, UA_Byte newAccessLevel)
    {
        return writeAttribute(nodeId, UA_ATTRIBUTEID_ACCESSLEVEL, &newAccessLevel, &UA_TYPES[UA_TYPES_BYTE]);
    }
    /*!
        \brief setUserAccessLevelAttribute
        \param nodeId
        \param newUserAccessLevel
        \return   true on success
    */
    bool setUserAccessLevelAttribute(NodeId& nodeId, UA_Byte newUserAccessLevel)
    {
        return writeAttribute(nodeId, UA_ATTRIBUTEID_USERACCESSLEVEL, &newUserAccessLevel, &UA_TYPES[UA_TYPES_BYTE]);
    }
    /*!
        \brief setMinimumSamplingIntervalAttribute
        \param nodeId
        \param newMinInterval
        \return   true on success
    */
    bool setMinimumSamplingIntervalAttribute(NodeId& nodeId, UA_Double newMinInterval)
    {
        return writeAttribute(nodeId,
                              UA_ATTRIBUTEID_MINIMUMSAMPLINGINTERVAL,
                              &newMinInterval,
                              &UA_TYPES[UA_TYPES_DOUBLE]);
    }
    /*!
        \brief setHistorizingAttribute
        \param nodeId
        \param newHistorizing
        \return   true on success
    */
    bool setHistorizingAttribute(NodeId& nodeId, UA_Boolean newHistorizing)
    {
        return writeAttribute(nodeId, UA_ATTRIBUTEID_HISTORIZING, &newHistorizing, &UA_TYPES[UA_TYPES_BOOLEAN]);
    }
    /*!
        \brief setExecutableAttribute
        \param nodeId
        \param newExecutable
        \return   true on success
    */
    bool setExecutableAttribute(NodeId& nodeId, UA_Boolean newExecutable)
    {
        _lastError = writeAttribute(nodeId, UA_ATTRIBUTEID_EXECUTABLE, &newExecutable, &UA_TYPES[UA_TYPES_BOOLEAN]);
        return lastOK();
    }
    /*!
        \brief setUserExecutableAttribute
        \param nodeId
        \param newUserExecutable
        \return   true on success
    */
    bool setUserExecutableAttribute(NodeId& nodeId, UA_Boolean newUserExecutable)
    {
        _lastError =
            writeAttribute(nodeId, UA_ATTRIBUTEID_USEREXECUTABLE, &newUserExecutable, &UA_TYPES[UA_TYPES_BOOLEAN]);
        return lastOK();
    }

    // End of attributes

    /*!
        \brief variable
        \param nodeId
        \param value
        \return   true on success
    */
    bool variable(NodeId& nodeId, Variant& value)
    {
        if (!_client)
            return false;
        WriteLock l(_mutex);
        // outValue is managed by caller - transfer to output value
        value.clear();
        _lastError = UA_Client_readValueAttribute(_client, nodeId, value);  // shallow copy
        return lastOK();
    }

    /*!
        \brief nodeClass
        \param nodeId
        \param c
        \return   true on success
    */
    bool nodeClass(NodeId& nodeId, NodeClass& c)
    {
        WriteLock l(_mutex);
        if (!_client)
            throw std::runtime_error("Null client");
        _lastError = UA_Client_readNodeClassAttribute(_client, nodeId, &c);
        return lastOK();
    }

    /*!
        \brief deleteNode
        \param nodeId
        \param deleteReferences
        \return   true on success
    */
    bool deleteNode(NodeId& nodeId, bool deleteReferences)
    {
        WriteLock l(_mutex);
        if (!_client)
            throw std::runtime_error("Null client");
        _lastError = UA_Client_deleteNode(_client, nodeId, UA_Boolean(deleteReferences));
        return lastOK();
    }

    /*!
        \brief deleteTree
        \param nodeId
        \return   true on success
    */
    bool deleteTree(NodeId& nodeId);  // recursive delete

    /*!
        \brief Client::deleteChildren
        \param n
    */
    void deleteChildren(UA_NodeId& n);
    /*!
        \brief callMethod
        \param objectId
        \param methodId
        \param in
        \param out
        \return   true on success
    */
    bool callMethod(NodeId& objectId, NodeId& methodId, VariantList& in, VariantCallResult& out)
    {
        WriteLock l(_mutex);
        size_t outputSize  = 0;
        UA_Variant* output = nullptr;
        if (!_client)
            throw std::runtime_error("Null client");
        _lastError = UA_STATUSCODE_GOOD;
        _lastError = UA_Client_call(_client, objectId, methodId, in.size(), in.data(), &outputSize, &output);
        if (_lastError == UA_STATUSCODE_GOOD) {
            out.set(output, outputSize);
        }
        return lastOK();
    }

    /*!
        \brief process
        \return   true on success
    */
    virtual bool process() { return true; }

    /*!
        \brief lastOK
        \return   true if last error is UA_STATUSCODE_GOOD
    */
    bool lastOK() const { return _lastError == UA_STATUSCODE_GOOD; }

    // Add nodes - templated from docs
    /*!
        \brief addVariableTypeNode
        \param requestedNewNodeId
        \param parentNodeId
        \param referenceTypeId
        \param browseName
        \param attr
        \param outNewNodeId
        \return true on success
    */
    bool addVariableTypeNode(NodeId& requestedNewNodeId,
                             NodeId& parentNodeId,
                             NodeId& referenceTypeId,
                             QualifiedName& browseName,
                             VariableTypeAttributes& attr,
                             NodeId& outNewNodeId = NodeId::Null)
    {
        if (!_client)
            return false;
        WriteLock l(_mutex);
        _lastError = UA_Client_addVariableTypeNode(_client,
                                                   requestedNewNodeId,
                                                   parentNodeId,
                                                   referenceTypeId,
                                                   browseName,
                                                   attr,
                                                   outNewNodeId.isNull() ? nullptr : outNewNodeId.ref());
        return lastOK();
    }
    /*!
        \brief addObjectNode
        \param requestedNewNodeId
        \param parentNodeId
        \param referenceTypeId
        \param browseName
        \param typeDefinition
        \param attr
        \param outNewNodeId
        \return true on success
    */
    bool addObjectNode(NodeId& requestedNewNodeId,
                       NodeId& parentNodeId,
                       NodeId& referenceTypeId,
                       QualifiedName& browseName,
                       NodeId& typeDefinition,
                       ObjectAttributes& attr,
                       NodeId& outNewNodeId = NodeId::Null)
    {
        if (!_client)
            return false;
        WriteLock l(_mutex);
        _lastError = UA_Client_addObjectNode(_client,
                                             requestedNewNodeId,
                                             parentNodeId,
                                             referenceTypeId,
                                             browseName,
                                             typeDefinition,
                                             attr,
                                             outNewNodeId.isNull() ? nullptr : outNewNodeId.ref());
        return lastOK();
    }
    /*!
        \brief addObjectTypeNode
        \param requestedNewNodeId
        \param parentNodeId
        \param referenceTypeId
        \param browseName
        \param attr
        \param outNewNodeId
        \return true on success
    */
    bool addObjectTypeNode(NodeId& requestedNewNodeId,
                           NodeId& parentNodeId,
                           NodeId& referenceTypeId,
                           QualifiedName& browseName,
                           ObjectTypeAttributes& attr,
                           NodeId& outNewNodeId = NodeId::Null)
    {
        if (!_client)
            return false;
        WriteLock l(_mutex);
        _lastError = UA_Client_addObjectTypeNode(_client,
                                                 requestedNewNodeId,
                                                 parentNodeId,
                                                 referenceTypeId,
                                                 browseName,
                                                 attr,
                                                 outNewNodeId.isNull() ? nullptr : outNewNodeId.ref());
        return lastOK();
    }
    /*!
        \brief addViewNode
        \param requestedNewNodeId
        \param parentNodeId
        \param referenceTypeId
        \param browseName
        \param attr
        \param outNewNodeId
        \return true on success
    */
    bool addViewNode(NodeId& requestedNewNodeId,
                     NodeId& parentNodeId,
                     NodeId& referenceTypeId,
                     QualifiedName& browseName,
                     ViewAttributes& attr,
                     NodeId& outNewNodeId = NodeId::Null)
    {
        if (!_client)
            return false;
        WriteLock l(_mutex);
        _lastError = UA_Client_addViewNode(_client,
                                           requestedNewNodeId,
                                           parentNodeId,
                                           referenceTypeId,
                                           browseName,
                                           attr,
                                           outNewNodeId.isNull() ? nullptr : outNewNodeId.ref());
        return lastOK();
    }
    /*!
        \brief addReferenceTypeNode
        \param requestedNewNodeId
        \param parentNodeId
        \param referenceTypeId
        \param browseName
        \param attr
        \param outNewNodeId
        \return true on success
    */
    bool addReferenceTypeNode(NodeId& requestedNewNodeId,
                              NodeId& parentNodeId,
                              NodeId& referenceTypeId,
                              QualifiedName& browseName,
                              ReferenceTypeAttributes& attr,
                              NodeId& outNewNodeId = NodeId::Null)
    {
        if (!_client)
            return false;
        WriteLock l(_mutex);
        _lastError = UA_Client_addReferenceTypeNode(_client,
                                                    requestedNewNodeId,
                                                    parentNodeId,
                                                    referenceTypeId,
                                                    browseName,
                                                    attr,
                                                    outNewNodeId.isNull() ? nullptr : outNewNodeId.ref());
        return lastOK();
    }
    /*!
        \brief addDataTypeNode
        \param requestedNewNodeId
        \param parentNodeId
        \param referenceTypeId
        \param browseName
        \param attr
        \param outNewNodeId
        \return true on success
    */
    bool addDataTypeNode(NodeId& requestedNewNodeId,
                         NodeId& parentNodeId,
                         NodeId& referenceTypeId,
                         QualifiedName& browseName,
                         DataTypeAttributes& attr,
                         NodeId& outNewNodeId = NodeId::Null)
    {
        if (!_client)
            return false;
        WriteLock l(_mutex);
        _lastError = UA_Client_addDataTypeNode(_client,
                                               requestedNewNodeId,
                                               parentNodeId,
                                               referenceTypeId,
                                               browseName,
                                               attr,
                                               outNewNodeId.isNull() ? nullptr : outNewNodeId.ref());
        return lastOK();
    }
    /*!
        \brief addMethodNode
        \param requestedNewNodeId
        \param parentNodeId
        \param referenceTypeId
        \param browseName
        \param attr
        \param outNewNodeId
        \return true on success
    */
    bool addMethodNode(NodeId& requestedNewNodeId,
                       NodeId& parentNodeId,
                       NodeId& referenceTypeId,
                       QualifiedName& browseName,
                       MethodAttributes& attr,
                       NodeId& outNewNodeId = NodeId::Null)
    {
        if (!_client)
            return false;
        WriteLock l(_mutex);
        _lastError = UA_Client_addMethodNode(_client,
                                             requestedNewNodeId,
                                             parentNodeId,
                                             referenceTypeId,
                                             browseName,
                                             attr,
                                             outNewNodeId.isNull() ? nullptr : outNewNodeId.ref());
        return lastOK();
    }

    /*!
        \brief addProperty
        \param parent
        \param key
        \param value
        \param nodeId
        \param newNode
        \return true on success
    */
    bool addProperty(NodeId& parent,
                     const std::string& key,
                     Variant& value,
                     NodeId& nodeId,
                     NodeId& newNode    = NodeId::Null,
                     int nameSpaceIndex = 0);

    //
    // Async services
    //
    /*!
        \brief asyncServiceCallback
        \param client
        \param userdata
        \param requestId
        \param response
        \param responseType
    */
    static void asyncServiceCallback(UA_Client* client,
                                     void* userdata,
                                     UA_UInt32 requestId,
                                     void* response,
                                     const UA_DataType* responseType);

    /*!
        \brief asyncService
        \param userdata
        \param requestId
        \param response
        \param responseType
    */
    virtual void asyncService(void* /*userdata*/,
                              UA_UInt32 /*requestId*/,
                              void* /*response*/,
                              const UA_DataType* /*responseType*/)
    {
    }
    /*!
        \brief historicalIterator
        \return
    */
    virtual bool historicalIterator(const NodeId& /*node*/,
                                    UA_Boolean /*moreDataAvailable*/,
                                    const UA_ExtensionObject& /*data*/)
    {
        return false;
    }
    /*!
        \brief historicalIteratorCallback
        \param client
        \param nodeId
        \param moreDataAvailable
        \param data
        \param callbackContext
        \return
    */
    static UA_Boolean historicalIteratorCallback(UA_Client* client,
                                                 const UA_NodeId* nodeId,
                                                 UA_Boolean moreDataAvailable,
                                                 const UA_ExtensionObject* data,
                                                 void* callbackContext)
    {
        if (callbackContext && nodeId && data) {
            Client* p = (Client*)callbackContext;
            NodeId n(*nodeId);
            return (p->historicalIterator(n, moreDataAvailable, *data)) ? UA_TRUE : UA_FALSE;
        }
        return UA_FALSE;
    }

    /*!
        \brief historyReadRaw
        \param n
        \param startTime
        \param endTime
        \param numValuesPerNode
        \param indexRange
        \param returnBounds
        \param timestampsToReturn
        \return
    */
    bool historyReadRaw(const NodeId& n,
                        UA_DateTime startTime,
                        UA_DateTime endTime,
                        unsigned numValuesPerNode,
                        const UA_String& indexRange              = UA_STRING_NULL,
                        bool returnBounds                        = false,
                        UA_TimestampsToReturn timestampsToReturn = UA_TIMESTAMPSTORETURN_BOTH)
    {
        _lastError = UA_Client_HistoryRead_raw(_client,
                                               n.constRef(),
                                               historicalIteratorCallback,
                                               startTime,
                                               endTime,
                                               indexRange,
                                               returnBounds ? UA_TRUE : UA_FALSE,
                                               (UA_UInt32)numValuesPerNode,
                                               timestampsToReturn,
                                               this);
        return lastOK();
    }
    /*!
        \brief historyUpdateInsert
        \param n
        \param value
        \return
    */
    bool historyUpdateInsert(const NodeId& n, const UA_DataValue& value)
    {

        _lastError = UA_Client_HistoryUpdate_insert(_client, n.constRef(), const_cast<UA_DataValue*>(&value));
        return lastOK();
    }
    /*!
        \brief historyUpdateReplace
        \param n
        \param value
        \return
    */
    bool historyUpdateReplace(const NodeId& n, const UA_DataValue& value)
    {

        _lastError = UA_Client_HistoryUpdate_replace(_client, n.constRef(), const_cast<UA_DataValue*>(&value));
        return lastOK();
    }
    /*!
        \brief historyUpdateUpdate
        \param n
        \param value
        \return
    */
    bool historyUpdateUpdate(const NodeId& n, const UA_DataValue& value)
    {

        _lastError = UA_Client_HistoryUpdate_update(_client, n.constRef(), const_cast<UA_DataValue*>(&value));
        return lastOK();
    }
    /*!
        \brief historyUpdateDeleteRaw
        \param n
        \param startTimestamp
        \param endTimestamp
        \return
    */
    bool historyUpdateDeleteRaw(const NodeId& n, UA_DateTime startTimestamp, UA_DateTime endTimestamp)
    {
        _lastError = UA_Client_HistoryUpdate_deleteRaw(_client, n.constRef(), startTimestamp, endTimestamp);
        return lastOK();
    }

    /*!
     * \brief findDataType
     * \param typeId
     * \return
     */
    const UA_DataType* findDataType(const UA_NodeId* typeId) { return UA_Client_findDataType(_client, typeId); }

    /*!
     * \brief addTimedCallback
     * \param data
     * \param date
     * \param callbackId
     * \return
     */
    bool addTimedEvent(unsigned msDelay, UA_UInt64& callbackId, std::function<void(Timer&)> func)
    {
        if (_client) {
            UA_DateTime date = UA_DateTime_nowMonotonic() + (UA_DATETIME_MSEC * msDelay);
            TimerPtr t(new Timer(this, 0, true, func));
            _lastError = UA_Client_addTimedCallback(_client, Client::clientCallback, t.get(), date, &callbackId);
            t->setId(callbackId);
            _timerMap[callbackId] = std::move(t);
            return lastOK();
        }
        callbackId = 0;
        return false;
    }

    /* Add a callback for cyclic repetition to the client.
     *
     * @param client The client object.
     * @param callback The callback that shall be added.
     * @param data Data that is forwarded to the callback.
     * @param interval_ms The callback shall be repeatedly executed with the given
     *        interval (in ms). The interval must be positive. The first execution
     *        occurs at now() + interval at the latest.
     * @param callbackId Set to the identifier of the repeated callback . This can
     *        be used to cancel the callback later on. If the pointer is null, the
     *        identifier is not set.
     * @return Upon success, UA_STATUSCODE_GOOD is returned. An error code
     *         otherwise. */

    bool addRepeatedTimerEvent(UA_Double interval_ms, UA_UInt64& callbackId, std::function<void(Timer&)> func)
    {
        if (_client) {
            TimerPtr t(new Timer(this, 0, false, func));
            _lastError =
                UA_Client_addRepeatedCallback(_client, Client::clientCallback, t.get(), interval_ms, &callbackId);
            t->setId(callbackId);
            _timerMap[callbackId] = std::move(t);
            return lastOK();
        }
        callbackId = 0;
        return false;
    }
    /*!
     * \brief changeRepeatedCallbackInterval
     * \param callbackId
     * \param interval_ms
     * \return
     */
    bool changeRepeatedTimerInterval(UA_UInt64 callbackId, UA_Double interval_ms)
    {
        if (_client) {
            _lastError = UA_Client_changeRepeatedCallbackInterval(_client, callbackId, interval_ms);
            return lastOK();
        }
        return false;
    }
    /*!
     * \brief UA_Client_removeCallback
     * \param client
     * \param callbackId
     */
    void removeTimerEvent(UA_UInt64 callbackId) { _timerMap.erase(callbackId); }

    // connection status - updated in call back
    UA_SecureChannelState getChannelState() const { return _channelState; }
    UA_SessionState getSessionState() const { return _sessionState; }
    UA_StatusCode getConnectStatus() const { return _connectStatus; }
};

}  // namespace Open62541

#endif  // OPEN62541CLIENT_H
