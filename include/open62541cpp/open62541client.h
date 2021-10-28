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

    UA_Client* client_or_throw() const
    {
        if (_client)
            return _client;
        throw StringException("Client is not initialized.");
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
    void runIterate(uint32_t interval = 100)
    {
        if (_client && (_connectStatus == UA_STATUSCODE_GOOD)) {
            throw_bad_status(UA_Client_run_iterate(_client, interval));
        }
        throw StringException("Client not connected");
    }

    /*!
     * \brief run
     * \return true on success
     */
    void run()
    {
        while (_connectStatus == UA_STATUSCODE_GOOD) {
            runIterate();
            process();
        }
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
    void getEndpoints(const std::string& serverUrl, EndpointDescriptionArray& list)
    {
        WriteLock l(_mutex);
        size_t endpointDescriptionsSize              = 0;
        UA_EndpointDescription* endpointDescriptions = nullptr;
        throw_bad_status(UA_Client_getEndpoints(client_or_throw(),
                                                serverUrl.c_str(),
                                                &endpointDescriptionsSize,
                                                &endpointDescriptions));
        // copy list so it is managed by the caller
        list.setList(endpointDescriptionsSize, endpointDescriptions);
    }

    /*!
        \brief findServers
        \param serverUrl
        \param serverUris
        \param localeIds
        \param registeredServers
        \return true on success
    */
    void findServers(const std::string& serverUrl,
                     StringArray& serverUris,
                     StringArray& localeIds,
                     ApplicationDescriptionArray& registeredServers)
    {
        WriteLock l(_mutex);
        throw_bad_status(UA_Client_findServers(client_or_throw(),
                                               serverUrl.c_str(),
                                               serverUris.length(),
                                               serverUris.data(),
                                               localeIds.length(),
                                               localeIds.data(),
                                               registeredServers.lengthRef(),
                                               registeredServers.dataRef()));
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
    void findServersOnNetwork(const std::string& serverUrl,
                              unsigned startingRecordId,
                              unsigned maxRecordsToReturn,
                              StringArray& serverCapabilityFilter,
                              ServerOnNetworkArray& serverOnNetwork)
    {
        WriteLock l(_mutex);
        throw_bad_status(UA_Client_findServersOnNetwork(client_or_throw(),
                                                        serverUrl.c_str(),
                                                        startingRecordId,
                                                        maxRecordsToReturn,
                                                        serverCapabilityFilter.length(),
                                                        serverCapabilityFilter.data(),
                                                        serverOnNetwork.lengthRef(),
                                                        serverOnNetwork.dataRef()));
    }
    /*!
        \brief readAttribute
        \param nodeId
        \param attributeId
        \param out
        \param outDataType
        \return true on success
    */
    void readAttribute(const UA_NodeId* nodeId, UA_AttributeId attributeId, void* out, const UA_DataType* outDataType)
    {
        WriteLock l(_mutex);
        throw_bad_status(__UA_Client_readAttribute(client_or_throw(), nodeId, attributeId, out, outDataType));
    }

    /*!
        \brief writeAttribute
        \param nodeId
        \param attributeId
        \param in
        \param inDataType
        \return true on success
    */
    void writeAttribute(const UA_NodeId* nodeId,
                        UA_AttributeId attributeId,
                        const void* in,
                        const UA_DataType* inDataType)
    {
        WriteLock l(_mutex);
        throw_bad_status(__UA_Client_writeAttribute(client_or_throw(), nodeId, attributeId, in, inDataType));
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
        UA_StatusCode c;
        UA_Client_getState(client_or_throw(), &channelState, &sessionState, &c);
        return c;
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

    //
    // Connect and Disconnect
    //
    /*!
        \brief connect
        \param endpointUrl
        \return true on success
    */
    void connect(const std::string& endpointUrl)
    {
        initialise();
        WriteLock l(_mutex);
        _connectionType = ConnectionType::NONE;
        throw_bad_status(UA_Client_connect(client_or_throw(), endpointUrl.c_str()));
        _connectionType = ConnectionType::CONNECTION;
    }

    /*!  Connect to the selected server with the given username and password

        @param client to use
        @param endpointURL to connect (for example "opc.tcp://localhost:16664")
        @param username
        @param password
        @return Indicates whether the operation succeeded or returns an error code */
    void connectUsername(const std::string& endpoint, const std::string& username, const std::string& password)
    {
        initialise();
        WriteLock l(_mutex);
        _connectionType = ConnectionType::NONE;
        throw_bad_status(
            UA_Client_connectUsername(client_or_throw(), endpoint.c_str(), username.c_str(), password.c_str()));
        _connectionType = ConnectionType::CONNECTION;
    }
    /*!
        \brief connectAsync
        \param endpoint
        \return Indicates whether the operation succeeded or returns an error code
    */
    void connectAsync(const std::string& endpoint)
    {
        initialise();
        WriteLock l(_mutex);
        _connectionType = ConnectionType::NONE;
        throw_bad_status(UA_Client_connectAsync(client_or_throw(), endpoint.c_str()));

        _connectionType = ConnectionType::ASYNC;
    }

    /*!
     * \brief connectSecureChannel
     * \param endpoint
     * \return Indicates whether the operation succeeded or returns an error code
     */
    void connectSecureChannel(const std::string& endpoint)
    {
        initialise();
        WriteLock l(_mutex);
        _connectionType = ConnectionType::NONE;
        throw_bad_status(UA_Client_connectSecureChannel(client_or_throw(), endpoint.c_str()));

        _connectionType = ConnectionType::SECURE;
    }

    /*!
     * \brief connectSecureChannelAsync
     * \param endpoint
     * \return Indicates whether the operation succeeded or returns an error code
     */
    void connectSecureChannelAsync(const std::string& endpoint)
    {
        initialise();
        WriteLock l(_mutex);
        _connectionType = ConnectionType::NONE;
        throw_bad_status(UA_Client_connectSecureChannelAsync(client_or_throw(), endpoint.c_str()));

        _connectionType = ConnectionType::SECUREASYNC;
    }

    /*!
        \brief disconnect
        \return
    */
    void disconnect()
    {
        WriteLock l(_mutex);
        // close subscriptions
        subscriptions().clear();
        _timerMap.clear();  // remove timer objects
        throw_bad_status(UA_Client_disconnect(client_or_throw()));
        _connectionType = ConnectionType::NONE;
    }
    /*!
        \brief disconnectAsync
        \return true on success
    */
    void disconnectAsync()
    {
        WriteLock l(_mutex);
        _timerMap.clear();  // remove timer objects
        throw_bad_status(UA_Client_disconnectAsync(client_or_throw()));
        _connectionType = ConnectionType::NONE;
    }

    void disconnectSecureChannel()
    {
        WriteLock l(_mutex);
        _timerMap.clear();  // remove timer objects
        if ((_connectionType == ConnectionType::SECURE) || (_connectionType == ConnectionType::SECUREASYNC)) {
            throw_bad_status(UA_Client_disconnectSecureChannel(client_or_throw()));
            _connectionType = ConnectionType::NONE;
        }
        else {
            throw std::runtime_error("Not a secure connection");
        }
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
    void getEndpoints(const std::string& serverUrl, std::vector<std::string>& list);

    /*!  Get the namespace-index of a namespace-URI

        @param client The UA_Client struct for this connection
        @param namespaceUri The interested namespace URI
        @param namespaceIndex The namespace index of the URI. The value is unchanged
              in case of an error
        @return Indicates whether the operation succeeded or returns an error code */
    int namespaceGetIndex(const std::string& namespaceUri)
    {
        WriteLock l(_mutex);
        int namespaceIndex = 0;
        UA_String s        = toUA_String(namespaceUri);
        if (UA_Client_NamespaceGetIndex(client_or_throw(), &s, (UA_UInt16*)(&namespaceIndex)) == UA_STATUSCODE_GOOD) {
            return namespaceIndex;
        }
        return -1;  // value
    }

    /*!
        \brief browseName
        \param nodeId
        \return true on success
    */
    void browseName(const NodeId& nodeId, std::string& s, int& ns)
    {
        WriteLock l(_mutex);
        QualifiedName outBrowseName;
        throw_bad_status(UA_Client_readBrowseNameAttribute(client_or_throw(), nodeId, outBrowseName));
        s  = toString(outBrowseName.get().name);
        ns = outBrowseName.get().namespaceIndex;
    }

    /*!
        \brief setBrowseName
        \param nodeId
        \param nameSpaceIndex
        \param name
    */
    void setBrowseName(const NodeId& nodeId, int nameSpaceIndex, const std::string& name)
    {
        WriteLock l(_mutex);
        QualifiedName newBrowseName(nameSpaceIndex, name);
        throw_bad_status(UA_Client_writeBrowseNameAttribute(client_or_throw(), nodeId, newBrowseName));
    }

    /*!
        \brief browseTree
        \param nodeId
        \param node
        \return true on success
    */
    void browseTree(const UA_NodeId& nodeId, Open62541::UANode* node);

    /*!
        \brief browseTree
        \param nodeId
        \return true on success
    */
    void browseTree(const NodeId& nodeId, UANodeTree& tree);
    /*!
        \brief browseTree
        \param nodeId
        \param tree
        \return
    */
    void browseTree(const NodeId& nodeId, UANode* tree);
    /*!
        \brief browseTree
        \param nodeId
        \param tree
        \return
    */
    void browseTree(const NodeId& nodeId, NodeIdMap& m);  // browse and create a map of string ids to node ids
    /*!
        \brief browseChildren
        \param nodeId
        \param m
        \return  true on success
    */
    void browseChildren(const UA_NodeId& nodeId, NodeIdMap& m);

    /*!
        \brief NodeIdFromPath get the node id from the path of browse names in the given namespace. Tests for node
       existance \param path \param nodeId \return  true on success
    */
    void nodeIdFromPath(const NodeId& start, const Path& path, NodeId& nodeId);

    /*!
        \brief createPath
        \param start
        \param path
        \param nameSpaceIndex
        \param nodeId
        \return  true on success
    */
    void createFolderPath(const NodeId& start, const Path& path, int nameSpaceIndex, NodeId& nodeId);

    /*!
        \brief getChild
        \param nameSpaceIndex
        \param childName
        \return
    */
    void getChild(const NodeId& start, const std::string& childName, NodeId& ret);

    /*!
        \brief addFolder
        \param parent
        \param nameSpaceIndex
        \param childName
        \return  true on success
    */
    void addFolder(const NodeId& parent,
                   const std::string& childName,
                   const NodeId& nodeId,
                   NodeId& newNode    = NodeId::Null,
                   int nameSpaceIndex = 0);

    /*!
        \brief addVariable
        \param parent
        \param nameSpaceIndex
        \param childName
        \return  true on success
    */
    void addVariable(const NodeId& parent,
                     const std::string& childName,
                     const Variant& value,
                     const NodeId& nodeId,
                     NodeId& newNode    = NodeId::Null,
                     int nameSpaceIndex = 0);

    /*!
        \brief setVariable
        \param nodeId
        \param value
        \return  true on success
    */
    void setVariable(const NodeId& nodeId, const Variant& value)
    {
        throw_bad_status(UA_Client_writeValueAttribute(client_or_throw(), nodeId, value));
    }

    // Attribute access generated from the docs
    /*!
        \brief readNodeIdAttribute
        \param nodeId
        \param outNodeId
        \return  true on success
    */
    void readNodeIdAttribute(const NodeId& nodeId, UA_NodeId& outNodeId)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_NODEID, &outNodeId, &UA_TYPES[UA_TYPES_NODEID]);
    }
    /*!
        \brief readNodeClassAttribute
        \param nodeId
        \param outNodeClass
        \return  true on success
    */
    void readNodeClassAttribute(const NodeId& nodeId, UA_NodeClass& outNodeClass)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_NODECLASS, &outNodeClass, &UA_TYPES[UA_TYPES_NODECLASS]);
    }
    /*!
        \brief readBrowseNameAttribute
        \param nodeId
        \param outBrowseName
        \return  true on success
    */
    void readBrowseNameAttribute(const NodeId& nodeId, QualifiedName& outBrowseName)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_BROWSENAME, outBrowseName, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
    }
    /*!
        \brief readDisplayNameAttribute
        \param nodeId
        \param outDisplayName
        \return  true on success
    */
    void readDisplayNameAttribute(const NodeId& nodeId, LocalizedText& outDisplayName)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_DISPLAYNAME, outDisplayName, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
    }
    /*!
        \brief readDescriptionAttribute
        \param nodeId
        \param outDescription
        \return  true on success
    */
    void readDescriptionAttribute(const NodeId& nodeId, LocalizedText& outDescription)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_DESCRIPTION, outDescription, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
    }
    /*!
        \brief readWriteMaskAttribute
        \param nodeId
        \param outWriteMask
        \return  true on success
    */
    void readWriteMaskAttribute(const NodeId& nodeId, UA_UInt32& outWriteMask)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_WRITEMASK, &outWriteMask, &UA_TYPES[UA_TYPES_UINT32]);
    }
    /*!
        \brief readUserWriteMaskAttribute
        \param nodeId
        \param outUserWriteMask
        \return  true on success
    */
    void readUserWriteMaskAttribute(const NodeId& nodeId, UA_UInt32& outUserWriteMask)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_USERWRITEMASK, &outUserWriteMask, &UA_TYPES[UA_TYPES_UINT32]);
    }
    /*!
        \brief readIsAbstractAttribute
        \param nodeId
        \param outIsAbstract
        \return  true on success
    */
    void readIsAbstractAttribute(const NodeId& nodeId, UA_Boolean& outIsAbstract)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_ISABSTRACT, &outIsAbstract, &UA_TYPES[UA_TYPES_BOOLEAN]);
    }
    /*!
        \brief readSymmetricAttribute
        \param nodeId
        \param outSymmetric
        \return  true on success
    */
    void readSymmetricAttribute(const NodeId& nodeId, UA_Boolean& outSymmetric)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_SYMMETRIC, &outSymmetric, &UA_TYPES[UA_TYPES_BOOLEAN]);
    }
    /*!
        \brief readInverseNameAttribute
        \param nodeId
        \param outInverseName
        \return  true on success
    */
    void readInverseNameAttribute(const NodeId& nodeId, LocalizedText& outInverseName)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_INVERSENAME, outInverseName, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
    }
    /*!
        \brief readContainsNoLoopsAttribute
        \param nodeId
        \param outContainsNoLoops
        \return  true on success
    */
    void readContainsNoLoopsAttribute(const NodeId& nodeId, UA_Boolean& outContainsNoLoops)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_CONTAINSNOLOOPS, &outContainsNoLoops, &UA_TYPES[UA_TYPES_BOOLEAN]);
    }
    /*!
        \brief readEventNotifierAttribute
        \param nodeId
        \param outEventNotifier
        \return  true on success
    */
    void readEventNotifierAttribute(const NodeId& nodeId, UA_Byte& outEventNotifier)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_EVENTNOTIFIER, &outEventNotifier, &UA_TYPES[UA_TYPES_BYTE]);
    }
    /*!
        \brief readValueAttribute
        \param nodeId
        \param outValue
        \return  true on success
    */
    void readValueAttribute(const NodeId& nodeId, Variant& outValue)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_VALUE, outValue, &UA_TYPES[UA_TYPES_VARIANT]);
    }
    /*!
        \brief readDataTypeAttribute
        \param nodeId
        \param outDataType
        \return  true on success
    */
    void readDataTypeAttribute(const NodeId& nodeId, UA_NodeId& outDataType)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_DATATYPE, &outDataType, &UA_TYPES[UA_TYPES_NODEID]);
    }
    /*!
        \brief readValueRankAttribute
        \param nodeId
        \param outValueRank
        \return  true on success
    */
    void readValueRankAttribute(const NodeId& nodeId, UA_Int32& outValueRank)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_VALUERANK, &outValueRank, &UA_TYPES[UA_TYPES_INT32]);
    }
    /*!
        \brief readArrayDimensionsAttribute
        \param nodeId
        \param ret
        \return true on success
    */
    void readArrayDimensionsAttribute(const NodeId& nodeId, std::vector<UA_UInt32>& ret)
    {
        WriteLock l(_mutex);
        size_t outArrayDimensionsSize;
        UA_UInt32* outArrayDimensions = nullptr;
        throw_bad_status(UA_Client_readArrayDimensionsAttribute(client_or_throw(),
                                                                nodeId,
                                                                &outArrayDimensionsSize,
                                                                &outArrayDimensions));

        if (outArrayDimensions) {
            for (int i = 0; i < int(outArrayDimensionsSize); i++) {
                ret.push_back(outArrayDimensions[i]);
            }
            UA_Array_delete(outArrayDimensions, outArrayDimensionsSize, &UA_TYPES[UA_TYPES_INT32]);
        }
    }
    /*!
        \brief readAccessLevelAttribute
        \param nodeId
        \param outAccessLevel
        \return  true on success
    */
    void readAccessLevelAttribute(const NodeId& nodeId, UA_Byte& outAccessLevel)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_ACCESSLEVEL, &outAccessLevel, &UA_TYPES[UA_TYPES_BYTE]);
    }
    /*!
        \brief readUserAccessLevelAttribute
        \param nodeId
        \param outUserAccessLevel
        \return  true on success
    */
    void readUserAccessLevelAttribute(const NodeId& nodeId, UA_Byte& outUserAccessLevel)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_USERACCESSLEVEL, &outUserAccessLevel, &UA_TYPES[UA_TYPES_BYTE]);
    }
    /*!
        \brief readMinimumSamplingIntervalAttribute
        \param nodeId
        \param outMinSamplingInterval
        \return  true on success
    */
    void readMinimumSamplingIntervalAttribute(const NodeId& nodeId, UA_Double& outMinSamplingInterval)
    {
        readAttribute(nodeId,
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
    void readHistorizingAttribute(const NodeId& nodeId, UA_Boolean& outHistorizing)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_HISTORIZING, &outHistorizing, &UA_TYPES[UA_TYPES_BOOLEAN]);
    }
    /*!
        \brief readExecutableAttribute
        \param nodeId
        \param outExecutable
        \return  true on success
    */
    void readExecutableAttribute(const NodeId& nodeId, UA_Boolean& outExecutable)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_EXECUTABLE, &outExecutable, &UA_TYPES[UA_TYPES_BOOLEAN]);
    }
    /*!
        \brief readUserExecutableAttribute
        \param nodeId
        \param outUserExecutable
        \return  true on success
    */
    void readUserExecutableAttribute(const NodeId& nodeId, UA_Boolean& outUserExecutable)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_USEREXECUTABLE, &outUserExecutable, &UA_TYPES[UA_TYPES_BOOLEAN]);
    }

    /*!
        \brief setNodeIdAttribute
        \param nodeId
        \param newNodeId
        \return  true on success
    */
    void setNodeIdAttribute(const NodeId& nodeId, const NodeId& newNodeId)
    {
        writeAttribute(nodeId, UA_ATTRIBUTEID_NODEID, &newNodeId, &UA_TYPES[UA_TYPES_NODEID]);
    }
    /*!
        \brief setNodeClassAttribute
        \param nodeId
        \param newNodeClass
        \return  true on success
    */
    void setNodeClassAttribute(const NodeId& nodeId, const UA_NodeClass& newNodeClass)
    {
        writeAttribute(nodeId, UA_ATTRIBUTEID_NODECLASS, &newNodeClass, &UA_TYPES[UA_TYPES_NODECLASS]);
    }
    /*!
        \brief setBrowseNameAttribute
        \param nodeId
        \param newBrowseName
        \return  true on success
    */
    void setBrowseNameAttribute(const NodeId& nodeId, const QualifiedName& newBrowseName)
    {
        writeAttribute(nodeId, UA_ATTRIBUTEID_BROWSENAME, &newBrowseName, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
    }
    /*!
        \brief setDisplayNameAttribute
        \param nodeId
        \param newDisplayName
        \return  true on success
    */
    void setDisplayNameAttribute(const NodeId& nodeId, const LocalizedText& newDisplayName)
    {
        writeAttribute(nodeId, UA_ATTRIBUTEID_DISPLAYNAME, &newDisplayName, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
    }
    /*!
        \brief setDescriptionAttribute
        \param nodeId
        \param newDescription
        \return  true on success
    */
    void setDescriptionAttribute(const NodeId& nodeId, const LocalizedText& newDescription)
    {
        writeAttribute(nodeId, UA_ATTRIBUTEID_DESCRIPTION, newDescription, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
    }
    /*!
        \brief setWriteMaskAttribute
        \param nodeId
        \param newWriteMask
        \return  true on success
    */
    void setWriteMaskAttribute(const NodeId& nodeId, UA_UInt32 newWriteMask)
    {
        writeAttribute(nodeId, UA_ATTRIBUTEID_WRITEMASK, &newWriteMask, &UA_TYPES[UA_TYPES_UINT32]);
    }
    /*!
        \brief setUserWriteMaskAttribute
        \param nodeId
        \param newUserWriteMask
        \return  true on success
    */
    void setUserWriteMaskAttribute(const NodeId& nodeId, UA_UInt32 newUserWriteMask)
    {
        writeAttribute(nodeId, UA_ATTRIBUTEID_USERWRITEMASK, &newUserWriteMask, &UA_TYPES[UA_TYPES_UINT32]);
    }
    /*!
        \brief setIsAbstractAttribute
        \param nodeId
        \param newIsAbstract
        \return  true on success
    */
    void setIsAbstractAttribute(const NodeId& nodeId, UA_Boolean newIsAbstract)
    {
        writeAttribute(nodeId, UA_ATTRIBUTEID_ISABSTRACT, &newIsAbstract, &UA_TYPES[UA_TYPES_BOOLEAN]);
    }
    /*!
        \brief setSymmetricAttribute
        \param nodeId
        \param newSymmetric
        \return  true on success
    */
    void setSymmetricAttribute(const NodeId& nodeId, UA_Boolean newSymmetric)
    {
        writeAttribute(nodeId, UA_ATTRIBUTEID_SYMMETRIC, &newSymmetric, &UA_TYPES[UA_TYPES_BOOLEAN]);
    }
    /*!
        \brief setInverseNameAttribute
        \param nodeId
        \param newInverseName
        \return  true on success
    */
    void setInverseNameAttribute(const NodeId& nodeId, const LocalizedText& newInverseName)
    {
        writeAttribute(nodeId, UA_ATTRIBUTEID_INVERSENAME, &newInverseName, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
    }
    /*!
        \brief setContainsNoLoopsAttribute
        \param nodeId
        \param newContainsNoLoops
        \return  true on success
    */
    void setContainsNoLoopsAttribute(const NodeId& nodeId, const UA_Boolean& newContainsNoLoops)
    {
        writeAttribute(nodeId, UA_ATTRIBUTEID_CONTAINSNOLOOPS, &newContainsNoLoops, &UA_TYPES[UA_TYPES_BOOLEAN]);
    }
    /*!
        \brief setEventNotifierAttribute
        \param nodeId
        \param newEventNotifier
        \return  true on success
    */
    void setEventNotifierAttribute(const NodeId& nodeId, UA_Byte newEventNotifier)
    {
        writeAttribute(nodeId, UA_ATTRIBUTEID_EVENTNOTIFIER, &newEventNotifier, &UA_TYPES[UA_TYPES_BYTE]);
    }
    /*!
        \brief setValueAttribute
        \param nodeId
        \param newValue
        \return  true on success
    */
    void setValueAttribute(const NodeId& nodeId, const Variant& newValue)
    {
        writeAttribute(nodeId, UA_ATTRIBUTEID_VALUE, newValue, &UA_TYPES[UA_TYPES_VARIANT]);
    }
    /*!
        \brief setDataTypeAttribute
        \param nodeId
        \param newDataType
        \return  true on success
    */
    void setDataTypeAttribute(const NodeId& nodeId, const UA_NodeId* newDataType)
    {
        writeAttribute(nodeId, UA_ATTRIBUTEID_DATATYPE, newDataType, &UA_TYPES[UA_TYPES_NODEID]);
    }
    /*!
        \brief setValueRankAttribute
        \param nodeId
        \param newValueRank
        \return   true on success
    */
    void setValueRankAttribute(const NodeId& nodeId, const UA_Int32 newValueRank)
    {
        writeAttribute(nodeId, UA_ATTRIBUTEID_VALUERANK, &newValueRank, &UA_TYPES[UA_TYPES_INT32]);
    }
    /*!
        \brief setArrayDimensionsAttribute
        \param nodeId
        \param newArrayDimensions
        \return   true on success
    */
    void setArrayDimensionsAttribute(const NodeId& nodeId, const std::vector<UA_UInt32>& newArrayDimensions)
    {
        UA_UInt32 v = newArrayDimensions.size();
        throw_bad_status(
            UA_Client_writeArrayDimensionsAttribute(client_or_throw(), nodeId, v, newArrayDimensions.data()));
    }
    /*!
        \brief setAccessLevelAttribute
        \param nodeId
        \param newAccessLevel
        \return   true on success
    */
    void setAccessLevelAttribute(const NodeId& nodeId, UA_Byte newAccessLevel)
    {
        writeAttribute(nodeId, UA_ATTRIBUTEID_ACCESSLEVEL, &newAccessLevel, &UA_TYPES[UA_TYPES_BYTE]);
    }
    /*!
        \brief setUserAccessLevelAttribute
        \param nodeId
        \param newUserAccessLevel
        \return   true on success
    */
    void setUserAccessLevelAttribute(const NodeId& nodeId, UA_Byte newUserAccessLevel)
    {
        writeAttribute(nodeId, UA_ATTRIBUTEID_USERACCESSLEVEL, &newUserAccessLevel, &UA_TYPES[UA_TYPES_BYTE]);
    }
    /*!
        \brief setMinimumSamplingIntervalAttribute
        \param nodeId
        \param newMinInterval
        \return   true on success
    */
    void setMinimumSamplingIntervalAttribute(const NodeId& nodeId, UA_Double newMinInterval)
    {
        writeAttribute(nodeId, UA_ATTRIBUTEID_MINIMUMSAMPLINGINTERVAL, &newMinInterval, &UA_TYPES[UA_TYPES_DOUBLE]);
    }
    /*!
        \brief setHistorizingAttribute
        \param nodeId
        \param newHistorizing
        \return   true on success
    */
    void setHistorizingAttribute(const NodeId& nodeId, UA_Boolean newHistorizing)
    {
        writeAttribute(nodeId, UA_ATTRIBUTEID_HISTORIZING, &newHistorizing, &UA_TYPES[UA_TYPES_BOOLEAN]);
    }
    /*!
        \brief setExecutableAttribute
        \param nodeId
        \param newExecutable
        \return   true on success
    */
    void setExecutableAttribute(const NodeId& nodeId, UA_Boolean newExecutable)
    {
        writeAttribute(nodeId, UA_ATTRIBUTEID_EXECUTABLE, &newExecutable, &UA_TYPES[UA_TYPES_BOOLEAN]);
    }
    /*!
        \brief setUserExecutableAttribute
        \param nodeId
        \param newUserExecutable
        \return   true on success
    */
    void setUserExecutableAttribute(const NodeId& nodeId, UA_Boolean newUserExecutable)
    {
        writeAttribute(nodeId, UA_ATTRIBUTEID_USEREXECUTABLE, &newUserExecutable, &UA_TYPES[UA_TYPES_BOOLEAN]);
    }

    // End of attributes

    /*!
        \brief variable
        \param nodeId
        \param value
        \return   true on success
    */
    void variable(const NodeId& nodeId, Variant& value)
    {
        WriteLock l(_mutex);
        // outValue is managed by caller - transfer to output value
        value.clear();
        throw_bad_status(UA_Client_readValueAttribute(client_or_throw(), nodeId, value));  // shallow copy
    }

    /*!
        \brief nodeClass
        \param nodeId
        \param c
        \return   true on success
    */
    void nodeClass(const NodeId& nodeId, NodeClass& c)
    {
        WriteLock l(_mutex);
        throw_bad_status(UA_Client_readNodeClassAttribute(client_or_throw(), nodeId, &c));
    }

    /*!
        \brief deleteNode
        \param nodeId
        \param deleteReferences
        \return   true on success
    */
    void deleteNode(const NodeId& nodeId, bool deleteReferences)
    {
        WriteLock l(_mutex);
        throw_bad_status(UA_Client_deleteNode(client_or_throw(), nodeId, UA_Boolean(deleteReferences)));
    }

    /*!
        \brief deleteTree
        \param nodeId
        \return   true on success
    */
    void deleteTree(const NodeId& nodeId);  // recursive delete

    /*!
        \brief Client::deleteChildren
        \param n
    */
    void deleteChildren(const UA_NodeId& n);
    /*!
        \brief callMethod
        \param objectId
        \param methodId
        \param in
        \param out
        \return   true on success
    */
    void callMethod(NodeId& objectId, NodeId& methodId, VariantList& in, VariantCallResult& out)
    {
        WriteLock l(_mutex);
        size_t outputSize  = 0;
        UA_Variant* output = nullptr;
        throw_bad_status(
            UA_Client_call(client_or_throw(), objectId, methodId, in.size(), in.data(), &outputSize, &output));
        out.set(output, outputSize);
    }

    /*!
        \brief process
        \return   true on success
    */
    virtual void process(){};

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
    void addVariableTypeNode(NodeId& requestedNewNodeId,
                             NodeId& parentNodeId,
                             NodeId& referenceTypeId,
                             QualifiedName& browseName,
                             VariableTypeAttributes& attr,
                             NodeId& outNewNodeId = NodeId::Null)
    {
        WriteLock l(_mutex);
        throw_bad_status(UA_Client_addVariableTypeNode(client_or_throw(),
                                                       requestedNewNodeId,
                                                       parentNodeId,
                                                       referenceTypeId,
                                                       browseName,
                                                       attr,
                                                       outNewNodeId.isNull() ? nullptr : outNewNodeId.ref()));
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
    void addObjectNode(NodeId& requestedNewNodeId,
                       NodeId& parentNodeId,
                       NodeId& referenceTypeId,
                       QualifiedName& browseName,
                       NodeId& typeDefinition,
                       ObjectAttributes& attr,
                       NodeId& outNewNodeId = NodeId::Null)
    {
        WriteLock l(_mutex);
        throw_bad_status(UA_Client_addObjectNode(client_or_throw(),
                                                 requestedNewNodeId,
                                                 parentNodeId,
                                                 referenceTypeId,
                                                 browseName,
                                                 typeDefinition,
                                                 attr,
                                                 outNewNodeId.isNull() ? nullptr : outNewNodeId.ref()));
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
    void addObjectTypeNode(NodeId& requestedNewNodeId,
                           NodeId& parentNodeId,
                           NodeId& referenceTypeId,
                           QualifiedName& browseName,
                           ObjectTypeAttributes& attr,
                           NodeId& outNewNodeId = NodeId::Null)
    {
        WriteLock l(_mutex);
        throw_bad_status(UA_Client_addObjectTypeNode(client_or_throw(),
                                                     requestedNewNodeId,
                                                     parentNodeId,
                                                     referenceTypeId,
                                                     browseName,
                                                     attr,
                                                     outNewNodeId.isNull() ? nullptr : outNewNodeId.ref()));
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
    void addViewNode(NodeId& requestedNewNodeId,
                     NodeId& parentNodeId,
                     NodeId& referenceTypeId,
                     QualifiedName& browseName,
                     ViewAttributes& attr,
                     NodeId& outNewNodeId = NodeId::Null)
    {
        WriteLock l(_mutex);
        throw_bad_status(UA_Client_addViewNode(client_or_throw(),
                                               requestedNewNodeId,
                                               parentNodeId,
                                               referenceTypeId,
                                               browseName,
                                               attr,
                                               outNewNodeId.isNull() ? nullptr : outNewNodeId.ref()));
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
    void addReferenceTypeNode(NodeId& requestedNewNodeId,
                              NodeId& parentNodeId,
                              NodeId& referenceTypeId,
                              QualifiedName& browseName,
                              ReferenceTypeAttributes& attr,
                              NodeId& outNewNodeId = NodeId::Null)
    {
        WriteLock l(_mutex);
        throw_bad_status(UA_Client_addReferenceTypeNode(client_or_throw(),
                                                        requestedNewNodeId,
                                                        parentNodeId,
                                                        referenceTypeId,
                                                        browseName,
                                                        attr,
                                                        outNewNodeId.isNull() ? nullptr : outNewNodeId.ref()));
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
    void addDataTypeNode(NodeId& requestedNewNodeId,
                         NodeId& parentNodeId,
                         NodeId& referenceTypeId,
                         QualifiedName& browseName,
                         DataTypeAttributes& attr,
                         NodeId& outNewNodeId = NodeId::Null)
    {
        WriteLock l(_mutex);
        throw_bad_status(UA_Client_addDataTypeNode(client_or_throw(),
                                                   requestedNewNodeId,
                                                   parentNodeId,
                                                   referenceTypeId,
                                                   browseName,
                                                   attr,
                                                   outNewNodeId.isNull() ? nullptr : outNewNodeId.ref()));
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
    void addMethodNode(NodeId& requestedNewNodeId,
                       NodeId& parentNodeId,
                       NodeId& referenceTypeId,
                       QualifiedName& browseName,
                       MethodAttributes& attr,
                       NodeId& outNewNodeId = NodeId::Null)
    {
        WriteLock l(_mutex);
        throw_bad_status(UA_Client_addMethodNode(client_or_throw(),
                                                 requestedNewNodeId,
                                                 parentNodeId,
                                                 referenceTypeId,
                                                 browseName,
                                                 attr,
                                                 outNewNodeId.isNull() ? nullptr : outNewNodeId.ref()));
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
    void addProperty(const NodeId& parent,
                     const std::string& key,
                     const Variant& value,
                     const NodeId& nodeId,
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
    void historyReadRaw(const NodeId& n,
                        UA_DateTime startTime,
                        UA_DateTime endTime,
                        unsigned numValuesPerNode,
                        const UA_String& indexRange              = UA_STRING_NULL,
                        bool returnBounds                        = false,
                        UA_TimestampsToReturn timestampsToReturn = UA_TIMESTAMPSTORETURN_BOTH)
    {
        throw_bad_status(UA_Client_HistoryRead_raw(client_or_throw(),
                                                   n.constRef(),
                                                   historicalIteratorCallback,
                                                   startTime,
                                                   endTime,
                                                   indexRange,
                                                   returnBounds ? UA_TRUE : UA_FALSE,
                                                   (UA_UInt32)numValuesPerNode,
                                                   timestampsToReturn,
                                                   this));
    }
    /*!
        \brief historyUpdateInsert
        \param n
        \param value
        \return
    */
    void historyUpdateInsert(const NodeId& n, const UA_DataValue& value)
    {

        throw_bad_status(
            UA_Client_HistoryUpdate_insert(client_or_throw(), n.constRef(), const_cast<UA_DataValue*>(&value)));
    }
    /*!
        \brief historyUpdateReplace
        \param n
        \param value
        \return
    */
    void historyUpdateReplace(const NodeId& n, const UA_DataValue& value)
    {

        throw_bad_status(
            UA_Client_HistoryUpdate_replace(client_or_throw(), n.constRef(), const_cast<UA_DataValue*>(&value)));
    }
    /*!
        \brief historyUpdateUpdate
        \param n
        \param value
        \return
    */
    void historyUpdateUpdate(const NodeId& n, const UA_DataValue& value)
    {

        throw_bad_status(
            UA_Client_HistoryUpdate_update(client_or_throw(), n.constRef(), const_cast<UA_DataValue*>(&value)));
    }
    /*!
        \brief historyUpdateDeleteRaw
        \param n
        \param startTimestamp
        \param endTimestamp
        \return
    */
    void historyUpdateDeleteRaw(const NodeId& n, UA_DateTime startTimestamp, UA_DateTime endTimestamp)
    {
        throw_bad_status(
            UA_Client_HistoryUpdate_deleteRaw(client_or_throw(), n.constRef(), startTimestamp, endTimestamp));
    }

    /*!
     * \brief findDataType
     * \param typeId
     * \return
     */
    const UA_DataType* findDataType(const UA_NodeId* typeId)
    {
        return UA_Client_findDataType(client_or_throw(), typeId);
    }

    /*!
     * \brief addTimedCallback
     * \param data
     * \param date
     * \param callbackId
     * \return
     */
    void addTimedEvent(unsigned msDelay, UA_UInt64& callbackId, std::function<void(Timer&)> func)
    {
        UA_DateTime date = UA_DateTime_nowMonotonic() + (UA_DATETIME_MSEC * msDelay);
        TimerPtr t(new Timer(this, 0, true, func));
        throw_bad_status(
            UA_Client_addTimedCallback(client_or_throw(), Client::clientCallback, t.get(), date, &callbackId));
        t->setId(callbackId);
        _timerMap[callbackId] = std::move(t);
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

    void addRepeatedTimerEvent(UA_Double interval_ms, UA_UInt64& callbackId, std::function<void(Timer&)> func)
    {
        TimerPtr t(new Timer(this, 0, false, func));
        throw_bad_status(UA_Client_addRepeatedCallback(client_or_throw(),
                                                       Client::clientCallback,
                                                       t.get(),
                                                       interval_ms,
                                                       &callbackId));
        t->setId(callbackId);
        _timerMap[callbackId] = std::move(t);
    }
    /*!
     * \brief changeRepeatedCallbackInterval
     * \param callbackId
     * \param interval_ms
     * \return
     */
    void changeRepeatedTimerInterval(UA_UInt64 callbackId, UA_Double interval_ms)
    {

        throw_bad_status(UA_Client_changeRepeatedCallbackInterval(client_or_throw(), callbackId, interval_ms));
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
