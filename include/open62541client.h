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
#include "open62541objects.h"
#include <clientsubscription.h>




/*
    OPC nodes are just data objects they do not need to be in a property tree
    nodes can be refered to by name or number (or GUID) which is  hash index to the item in the server
*/


namespace Open62541 {

    // Only really for receiving lists  not safe to copy
    class  UA_EXPORT  ApplicationDescriptionList : public std::vector<UA_ApplicationDescription *> {
        public:
            ApplicationDescriptionList() {}
            ~ApplicationDescriptionList() {
                for (auto i : *this) {
                    if (i) {
                        UA_ApplicationDescription_delete(i); // delete the item
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
        The main thing to watch for is Node ID objects are passed by reference. There are stock Node Id objects including NodeId::Null
        Pass NodeId::Null where a NULL UA_NodeId pointer is expected.
        If a NodeId is being passed to receive a value use the notNull() method to mark it as a receiver of a new node id.
        Most functions return true if the lastError is UA_STATUSCODE_GOOD.
    */

    class Client {
            UA_ClientConfig _config = UA_ClientConfig_default;
            UA_Client *_client = nullptr;
            ReadWriteMutex _mutex;
            //
            ClientSubscriptionMap _subscriptions;


        protected:
            UA_StatusCode _lastError = 0;


        private:
            // Call Backs
            static void  stateCallback(UA_Client *client, UA_ClientState clientState);
        public:

            Client() {
                _config.clientContext = this;
                _config.stateCallback = stateCallback;
                _config.subscriptionInactivityCallback = subscriptionInactivityCallback;
                _client = UA_Client_new(_config);
            }
            /*!
                \brief Open62541Client
                \param config
            */
            Client(UA_ClientConfig &config) {
                _config = config;
                _config.clientContext = this;
                _client = UA_Client_new(config);
            }

            /*!
                \brief ~Open62541Client
            */
            virtual ~Client() {
                if (_client)  disconnect();
            }

            /*!
                \brief runAsync
                \param interval
            */
            void runAsync(int interval) {
                UA_Client_runAsync(_client, interval);
            }
            /*!
                \brief getContext
                \return
            */
            void *getContext() {
                return UA_Client_getContext(client());
            }

            /*!
                \brief subscriptionInactivityCallback
                \param client
                \param subscriptionId
                \param subContext
            */
            static  void subscriptionInactivityCallback(UA_Client *client, UA_UInt32 subscriptionId, void *subContext);
            /*!
                \brief subscriptionInactivity
                \param subscriptionId
                \param subContext
            */
            virtual void subscriptionInactivity(UA_UInt32 /*subscriptionId*/, void * /*subContext*/) {}

            /*!
                \brief subscriptions
                \return map of subscriptions
            */
            ClientSubscriptionMap &subscriptions() {
                return  _subscriptions;
            }
            /*!
                \brief addSubscription
                \param newId receives Id of created subscription
                \return true on success
            */
            bool addSubscription(UA_UInt32 &newId, CreateSubscriptionRequest *settings = nullptr) {
                //
                ClientSubscriptionRef c(new ClientSubscription(*this));
                //
                if (settings) {
                    c->settings() = *settings; // assign settings across
                }
                //
                if (c->create()) {
                    newId = c->id();
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
            bool removeSubscription(UA_UInt32 Id) {
                subscriptions().erase(Id); // remove from dictionary implicit delete
                return true;
            }

            /*!
                \brief subscription
                \param Id
                \return pointer to subscription object or null
            */
            ClientSubscription *subscription(UA_UInt32 Id) {
                if (subscriptions().find(Id) != subscriptions().end()) {
                    ClientSubscriptionRef &c = subscriptions()[Id];
                    return c.get();
                }
                return nullptr;
            }

            //
            // Connection state handlers
            //
            /*!
                \brief stateDisconnected
            */
            virtual void stateDisconnected() {
                OPEN62541_TRC;
            }

            /*!
                \brief stateConnected
            */
            virtual void stateConnected() {
                OPEN62541_TRC;
            }

            /*!
                \brief stateSecureChannel
            */
            virtual void stateSecureChannel() {
                OPEN62541_TRC;
            }

            /*!
                \brief stateSession
            */
            virtual void stateSession() {
                OPEN62541_TRC;
            }

            /*!
                \brief stateSessionRenewed
            */
            virtual void stateSessionRenewed() {
                OPEN62541_TRC;
            }

            /*!
                \brief stateChange
                \param clientState
            */
            virtual void stateChange(UA_ClientState clientState) {
                switch (clientState) {
                    case UA_CLIENTSTATE_DISCONNECTED:
                        stateDisconnected();
                        break;
                    case UA_CLIENTSTATE_CONNECTED:
                        stateConnected();
                        break;
                    case UA_CLIENTSTATE_SECURECHANNEL:
                        stateSecureChannel();
                        break;
                    case UA_CLIENTSTATE_SESSION:
                        stateSession();
                        break;
                    case UA_CLIENTSTATE_SESSION_RENEWED:
                        stateSessionRenewed();
                        break;
                }
            }


            /*!
                \brief getEndpoints
                Retrive end points
                \param serverUrl
                \param list
                \return true on success
            */
            bool getEndpoints(const std::string &serverUrl, EndpointDescriptionArray &list) {
                WriteLock l(_mutex);
                size_t endpointDescriptionsSize = 0;
                UA_EndpointDescription *endpointDescriptions = nullptr;
                _lastError = UA_Client_getEndpoints(_client, serverUrl.c_str(),
                                                    &endpointDescriptionsSize,
                                                    &endpointDescriptions);
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
            bool findServers(const std::string &serverUrl,
                             StringArray &serverUris,
                             StringArray &localeIds,
                             ApplicationDescriptionArray &registeredServers) {
                WriteLock l(_mutex);
                _lastError = UA_Client_findServers(_client, serverUrl.c_str(),
                                                   serverUris.length(), serverUris.data(),
                                                   localeIds.length(), localeIds.data(),
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
            bool findServersOnNetwork(const std::string &serverUrl, unsigned startingRecordId,
                                      unsigned maxRecordsToReturn, StringArray &serverCapabilityFilter,
                                      ServerOnNetworkArray &serverOnNetwork) {
                WriteLock l(_mutex);
                _lastError =
                    UA_Client_findServersOnNetwork(_client, serverUrl.c_str(),
                                                   startingRecordId,  maxRecordsToReturn,
                                                   serverCapabilityFilter.length(), serverCapabilityFilter.data(),
                                                   serverOnNetwork.lengthRef(), serverOnNetwork.dataRef());
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
            bool readAttribute(const UA_NodeId *nodeId,  UA_AttributeId attributeId, void *out, const UA_DataType *outDataType) {
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
            bool writeAttribute(const UA_NodeId *nodeId, UA_AttributeId attributeId, const void *in,  const UA_DataType *inDataType) {
                WriteLock l(_mutex);
                _lastError = __UA_Client_writeAttribute(_client, nodeId, attributeId, in, inDataType);
                return lastOK();
            }

            /*!
             * \brief mutex
             * \return  client read/write mutex
             */
            ReadWriteMutex &mutex() {
                return _mutex;
            }
            /*!
                \brief getState
                \return connection state
            */
            UA_ClientState getState() {
                ReadLock l(_mutex);
                if (_client) return UA_Client_getState(_client);
                throw std::runtime_error("Null client");
                return  UA_CLIENTSTATE_DISCONNECTED;
            }

            /*!
                \brief reset
            */
            void reset() {
                WriteLock l(_mutex);
                if (_client) UA_Client_reset(_client);
                throw std::runtime_error("Null client");
                return;
            }

            /*!
                \brief client
                \return underlying client object
            */
            UA_Client *client() {
                ReadLock l(_mutex);
                return _client;
            }
            //
            /*!
                \brief config
                \return client configuration
            */
            UA_ClientConfig &config() {
                return _config;
            }
            /*!
                \brief lastError
                \return last error set
            */

            UA_StatusCode lastError() {
                return  _lastError;
            }
            //
            // Connect and Disconnect
            //
            /*!
             * \brief connect
             * \param endpointUrl
             * \return true on success
             */
            bool connect(const std::string &endpointUrl) {
                WriteLock l(_mutex);
                _lastError = UA_Client_connect(_client, endpointUrl.c_str());
                return lastOK();
            }

            /*!  Connect to the selected server with the given username and password

                @param client to use
                @param endpointURL to connect (for example "opc.tcp://localhost:16664")
                @param username
                @param password
                @return Indicates whether the operation succeeded or returns an error code */
            bool connectUsername(const std::string &endpoint, const std::string &username, const std::string &password) {
                WriteLock l(_mutex);
                if (!_client)throw std::runtime_error("Null client");
                _lastError = UA_Client_connect_username(_client, endpoint.c_str(), username.c_str(), password.c_str());

                return lastOK();
            }

            /*!
             * \brief disconnect
             * \return
             */
            UA_StatusCode disconnect() {
                WriteLock l(_mutex);
                if (!_client) throw std::runtime_error("Null client");
                _lastError = UA_Client_disconnect(_client);
                UA_Client_delete(_client);
                _client = nullptr;
                return lastOK();
            }
            /*!
             * \brief manuallyRenewSecureChannel
             * \return
             */
            bool manuallyRenewSecureChannel() {
                ReadLock l(_mutex);
                if (!_client) throw std::runtime_error("Null client");
                return (_lastError = UA_Client_manuallyRenewSecureChannel(_client)) == UA_STATUSCODE_GOOD;
            }

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
            UA_StatusCode getEndpoints(const std::string &serverUrl, std::vector<std::string> &list);

            /*!  Get the namespace-index of a namespace-URI

                @param client The UA_Client struct for this connection
                @param namespaceUri The interested namespace URI
                @param namespaceIndex The namespace index of the URI. The value is unchanged
                      in case of an error
                @return Indicates whether the operation succeeded or returns an error code */
            int  namespaceGetIndex(const std::string &namespaceUri) {
                WriteLock l(_mutex);
                if (!_client) throw std::runtime_error("Null client");
                int namespaceIndex = 0;
                UA_String s = toUA_String(namespaceUri);
                if (UA_Client_NamespaceGetIndex(_client, &s, (UA_UInt16 *)(&namespaceIndex)) == UA_STATUSCODE_GOOD) {
                    return namespaceIndex;
                }
                return -1; // value
            }

            /*!
                \brief browseName
                \param nodeId
                \return true on success
            */
            bool  browseName(NodeId &nodeId, std::string &s, int &ns) {
                WriteLock l(_mutex);
                if (!_client) throw std::runtime_error("Null client");
                QualifiedName outBrowseName;
                if ((_lastError = UA_Client_readBrowseNameAttribute(_client, nodeId, outBrowseName)) == UA_STATUSCODE_GOOD) {
                    s =   toString(outBrowseName.get().name);
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
            void setBrowseName(NodeId &nodeId, int nameSpaceIndex, const std::string &name) {
                WriteLock l(_mutex);
                if (!_client) throw std::runtime_error("Null client");
                QualifiedName newBrowseName(nameSpaceIndex, name);
                UA_Client_writeBrowseNameAttribute(_client, nodeId, newBrowseName);
            }

            /*!
                \brief browseTree
                \param nodeId
                \param node
                \return true on success
            */
            bool browseTree(UA_NodeId &nodeId, Open62541::UANode *node);

            /*!
                \brief browseTree
                \param nodeId
                \return true on success
            */
            bool browseTree(NodeId &nodeId, UANodeTree &tree);
            /*!
                \brief browseTree
                \param nodeId
                \param tree
                \return
            */
            bool browseTree(NodeId &nodeId, UANode *tree);
            /*!
                \brief browseTree
                \param nodeId
                \param tree
                \return
            */
            bool browseTree(NodeId &nodeId, NodeIdMap &m); // browse and create a map of string ids to node ids
            /*!
                \brief browseChildren
                \param nodeId
                \param m
                \return  true on success
            */
            bool browseChildren(UA_NodeId &nodeId, NodeIdMap &m);

            /*!
                \brief NodeIdFromPath get the node id from the path of browse names in the given namespace. Tests for node existance
                \param path
                \param nodeId
                \return  true on success
            */
            bool nodeIdFromPath(NodeId &start, Path &path,  NodeId &nodeId);

            /*!
                \brief createPath
                \param start
                \param path
                \param nameSpaceIndex
                \param nodeId
                \return  true on success
            */
            bool createFolderPath(NodeId &start, Path &path, int nameSpaceIndex, NodeId &nodeId);

            /*!
                \brief getChild
                \param nameSpaceIndex
                \param childName
                \return
            */
            bool getChild(NodeId &start,  const std::string &childName, NodeId &ret);

            /*!
                \brief addFolder
                \param parent
                \param nameSpaceIndex
                \param childName
                \return  true on success
            */
            bool addFolder(NodeId &parent,  const std::string &childName,
                           NodeId &nodeId, NodeId &newNode = NodeId::Null, int nameSpaceIndex = 0);

            /*!
                \brief addVariable
                \param parent
                \param nameSpaceIndex
                \param childName
                \return  true on success
            */
            bool addVariable(NodeId &parent, const std::string &childName, Variant &value,
                             NodeId &nodeId, NodeId &newNode = NodeId::Null, int nameSpaceIndex = 0);

            /*!
                \brief setVariable
                \param nodeId
                \param value
                \return  true on success
            */
            bool  setVariable(NodeId &nodeId,  Variant &value) {
                _lastError = UA_Client_writeValueAttribute(_client,  nodeId, value);
                return lastOK();
            }

            // Attribute access generated from the docs
            /*!
                \brief readNodeIdAttribute
                \param nodeId
                \param outNodeId
                \return  true on success
            */
            bool
            readNodeIdAttribute(NodeId &nodeId,
                                UA_NodeId &outNodeId) {
                return   readAttribute(nodeId, UA_ATTRIBUTEID_NODEID,
                                       &outNodeId, &UA_TYPES[UA_TYPES_NODEID]);
            }
            /*!
                \brief readNodeClassAttribute
                \param nodeId
                \param outNodeClass
                \return  true on success
            */
            bool
            readNodeClassAttribute(NodeId &nodeId,
                                   UA_NodeClass &outNodeClass) {
                return   readAttribute(nodeId, UA_ATTRIBUTEID_NODECLASS,
                                       &outNodeClass, &UA_TYPES[UA_TYPES_NODECLASS]);
            }
            /*!
                \brief readBrowseNameAttribute
                \param nodeId
                \param outBrowseName
                \return  true on success
            */
            bool
            readBrowseNameAttribute(NodeId &nodeId,
                                    QualifiedName &outBrowseName) {
                return   readAttribute(nodeId, UA_ATTRIBUTEID_BROWSENAME,
                                       outBrowseName,
                                       &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
            }
            /*!
                \brief readDisplayNameAttribute
                \param nodeId
                \param outDisplayName
                \return  true on success
            */
            bool
            readDisplayNameAttribute(NodeId &nodeId,
                                     LocalizedText &outDisplayName) {
                return   readAttribute(nodeId, UA_ATTRIBUTEID_DISPLAYNAME,
                                       outDisplayName,
                                       &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);

            }
            /*!
                \brief readDescriptionAttribute
                \param nodeId
                \param outDescription
                \return  true on success
            */
            bool
            readDescriptionAttribute(NodeId &nodeId,
                                     LocalizedText &outDescription) {
                return   readAttribute(nodeId, UA_ATTRIBUTEID_DESCRIPTION,
                                       outDescription,
                                       &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
                return lastOK();
            }
            /*!
                \brief readWriteMaskAttribute
                \param nodeId
                \param outWriteMask
                \return  true on success
            */
            bool
            readWriteMaskAttribute(NodeId &nodeId,
                                   UA_UInt32 &outWriteMask) {
                return readAttribute(nodeId, UA_ATTRIBUTEID_WRITEMASK,
                                     &outWriteMask, &UA_TYPES[UA_TYPES_UINT32]);

            }
            /*!
                \brief readUserWriteMaskAttribute
                \param nodeId
                \param outUserWriteMask
                \return  true on success
            */
            bool
            readUserWriteMaskAttribute(NodeId &nodeId,
                                       UA_UInt32 &outUserWriteMask) {
                return readAttribute(nodeId,
                                     UA_ATTRIBUTEID_USERWRITEMASK,
                                     &outUserWriteMask,
                                     &UA_TYPES[UA_TYPES_UINT32]);

            }
            /*!
                \brief readIsAbstractAttribute
                \param nodeId
                \param outIsAbstract
                \return  true on success
            */
            bool
            readIsAbstractAttribute(NodeId &nodeId,
                                    UA_Boolean &outIsAbstract) {
                return readAttribute(nodeId, UA_ATTRIBUTEID_ISABSTRACT,
                                     &outIsAbstract, &UA_TYPES[UA_TYPES_BOOLEAN]);

            }
            /*!
                \brief readSymmetricAttribute
                \param nodeId
                \param outSymmetric
                \return  true on success
            */
            bool
            readSymmetricAttribute(NodeId &nodeId,
                                   UA_Boolean &outSymmetric) {
                return readAttribute(nodeId, UA_ATTRIBUTEID_SYMMETRIC,
                                     &outSymmetric, &UA_TYPES[UA_TYPES_BOOLEAN]);

            }
            /*!
                \brief readInverseNameAttribute
                \param nodeId
                \param outInverseName
                \return  true on success
            */
            bool
            readInverseNameAttribute(NodeId &nodeId,
                                     LocalizedText &outInverseName) {
                return readAttribute(nodeId, UA_ATTRIBUTEID_INVERSENAME,
                                     outInverseName,
                                     &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);

            }
            /*!
                \brief readContainsNoLoopsAttribute
                \param nodeId
                \param outContainsNoLoops
                \return  true on success
            */
            bool
            readContainsNoLoopsAttribute(NodeId &nodeId,
                                         UA_Boolean &outContainsNoLoops) {
                return readAttribute(nodeId,
                                     UA_ATTRIBUTEID_CONTAINSNOLOOPS,
                                     &outContainsNoLoops,
                                     &UA_TYPES[UA_TYPES_BOOLEAN]);

            }
            /*!
                \brief readEventNotifierAttribute
                \param nodeId
                \param outEventNotifier
                \return  true on success
            */
            bool
            readEventNotifierAttribute(NodeId &nodeId,
                                       UA_Byte &outEventNotifier) {
                return readAttribute(nodeId, UA_ATTRIBUTEID_EVENTNOTIFIER,
                                     &outEventNotifier, &UA_TYPES[UA_TYPES_BYTE]);
            }
            /*!
                \brief readValueAttribute
                \param nodeId
                \param outValue
                \return  true on success
            */
            bool
            readValueAttribute(NodeId &nodeId,
                               Variant &outValue) {
                return readAttribute(nodeId, UA_ATTRIBUTEID_VALUE,
                                     outValue, &UA_TYPES[UA_TYPES_VARIANT]);
            }
            /*!
                \brief readDataTypeAttribute
                \param nodeId
                \param outDataType
                \return  true on success
            */
            bool
            readDataTypeAttribute(NodeId &nodeId,
                                  UA_NodeId &outDataType) {
                return readAttribute(nodeId, UA_ATTRIBUTEID_DATATYPE,
                                     &outDataType, &UA_TYPES[UA_TYPES_NODEID]);
            }
            /*!
                \brief readValueRankAttribute
                \param nodeId
                \param outValueRank
                \return  true on success
            */
            bool
            readValueRankAttribute(NodeId &nodeId,
                                   UA_Int32 &outValueRank) {
                return readAttribute(nodeId, UA_ATTRIBUTEID_VALUERANK,
                                     &outValueRank, &UA_TYPES[UA_TYPES_INT32]);
            }
            /*!
                \brief readArrayDimensionsAttribute
                \param nodeId
                \param ret
                \return true on success
            */
            bool readArrayDimensionsAttribute(NodeId &nodeId, std::vector<UA_UInt32> &ret) {
                WriteLock l(_mutex);
                size_t outArrayDimensionsSize;
                UA_UInt32 *outArrayDimensions = nullptr;
                _lastError = UA_Client_readArrayDimensionsAttribute(_client, nodeId, &outArrayDimensionsSize, &outArrayDimensions);
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
            bool
            readAccessLevelAttribute(NodeId &nodeId,
                                     UA_Byte &outAccessLevel) {
                return readAttribute(nodeId, UA_ATTRIBUTEID_ACCESSLEVEL,
                                     &outAccessLevel, &UA_TYPES[UA_TYPES_BYTE]);

            }
            /*!
                \brief readUserAccessLevelAttribute
                \param nodeId
                \param outUserAccessLevel
                \return  true on success
            */
            bool
            readUserAccessLevelAttribute(NodeId &nodeId,
                                         UA_Byte &outUserAccessLevel) {
                return readAttribute(nodeId,
                                     UA_ATTRIBUTEID_USERACCESSLEVEL,
                                     &outUserAccessLevel,
                                     &UA_TYPES[UA_TYPES_BYTE]);

            }
            /*!
                \brief readMinimumSamplingIntervalAttribute
                \param nodeId
                \param outMinSamplingInterval
                \return  true on success
            */
            bool
            readMinimumSamplingIntervalAttribute(NodeId &nodeId,
                                                 UA_Double &outMinSamplingInterval) {
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
            bool
            readHistorizingAttribute(NodeId &nodeId,
                                     UA_Boolean &outHistorizing) {
                return readAttribute(nodeId, UA_ATTRIBUTEID_HISTORIZING,
                                     &outHistorizing, &UA_TYPES[UA_TYPES_BOOLEAN]);

            }
            /*!
                \brief readExecutableAttribute
                \param nodeId
                \param outExecutable
                \return  true on success
            */
            bool
            readExecutableAttribute(NodeId &nodeId,
                                    UA_Boolean &outExecutable) {
                return readAttribute(nodeId, UA_ATTRIBUTEID_EXECUTABLE,
                                     &outExecutable, &UA_TYPES[UA_TYPES_BOOLEAN]);

            }
            /*!
                \brief readUserExecutableAttribute
                \param nodeId
                \param outUserExecutable
                \return  true on success
            */
            bool
            readUserExecutableAttribute(NodeId &nodeId,
                                        UA_Boolean &outUserExecutable) {
                return readAttribute(nodeId,
                                     UA_ATTRIBUTEID_USEREXECUTABLE,
                                     &outUserExecutable,
                                     &UA_TYPES[UA_TYPES_BOOLEAN]);
            }

            /*!
                \brief setNodeIdAttribute
                \param nodeId
                \param newNodeId
                \return  true on success
            */
            bool
            setNodeIdAttribute(NodeId &nodeId,
                               NodeId &newNodeId) {
                return   writeAttribute(nodeId, UA_ATTRIBUTEID_NODEID,
                                        &newNodeId, &UA_TYPES[UA_TYPES_NODEID]);
            }
            /*!
                \brief setNodeClassAttribute
                \param nodeId
                \param newNodeClass
                \return  true on success
            */
            bool
            setNodeClassAttribute(NodeId &nodeId,
                                  UA_NodeClass &newNodeClass) {
                return   writeAttribute(nodeId, UA_ATTRIBUTEID_NODECLASS,
                                        &newNodeClass, &UA_TYPES[UA_TYPES_NODECLASS]);
            }
            /*!
                \brief setBrowseNameAttribute
                \param nodeId
                \param newBrowseName
                \return  true on success
            */
            bool
            setBrowseNameAttribute(NodeId &nodeId,
                                   QualifiedName &newBrowseName) {
                return   writeAttribute(nodeId, UA_ATTRIBUTEID_BROWSENAME,
                                        &newBrowseName,
                                        &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
            }
            /*!
                \brief setDisplayNameAttribute
                \param nodeId
                \param newDisplayName
                \return  true on success
            */
            bool
            setDisplayNameAttribute(NodeId &nodeId,
                                    LocalizedText &newDisplayName) {
                return   writeAttribute(nodeId, UA_ATTRIBUTEID_DISPLAYNAME,
                                        &newDisplayName,
                                        &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
            }
            /*!
                \brief setDescriptionAttribute
                \param nodeId
                \param newDescription
                \return  true on success
            */
            bool
            setDescriptionAttribute(NodeId &nodeId,
                                    LocalizedText &newDescription) {
                return   writeAttribute(nodeId, UA_ATTRIBUTEID_DESCRIPTION,
                                        newDescription,
                                        &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
            }
            /*!
                \brief setWriteMaskAttribute
                \param nodeId
                \param newWriteMask
                \return  true on success
            */
            bool
            setWriteMaskAttribute(NodeId &nodeId,
                                  UA_UInt32 newWriteMask) {
                return   writeAttribute(nodeId, UA_ATTRIBUTEID_WRITEMASK,
                                        &newWriteMask, &UA_TYPES[UA_TYPES_UINT32]);
            }
            /*!
                \brief setUserWriteMaskAttribute
                \param nodeId
                \param newUserWriteMask
                \return  true on success
            */
            bool
            setUserWriteMaskAttribute(NodeId &nodeId,
                                      UA_UInt32 newUserWriteMask) {
                return   writeAttribute(nodeId,
                                        UA_ATTRIBUTEID_USERWRITEMASK,
                                        &newUserWriteMask,
                                        &UA_TYPES[UA_TYPES_UINT32]);
            }
            /*!
                \brief setIsAbstractAttribute
                \param nodeId
                \param newIsAbstract
                \return  true on success
            */
            bool
            setIsAbstractAttribute(NodeId &nodeId,
                                   UA_Boolean newIsAbstract) {
                return   writeAttribute(nodeId, UA_ATTRIBUTEID_ISABSTRACT,
                                        &newIsAbstract, &UA_TYPES[UA_TYPES_BOOLEAN]);
            }
            /*!
                \brief setSymmetricAttribute
                \param nodeId
                \param newSymmetric
                \return  true on success
            */
            bool
            setSymmetricAttribute(NodeId &nodeId,
                                  UA_Boolean newSymmetric) {
                return   writeAttribute(nodeId, UA_ATTRIBUTEID_SYMMETRIC,
                                        &newSymmetric, &UA_TYPES[UA_TYPES_BOOLEAN]);
            }
            /*!
                \brief setInverseNameAttribute
                \param nodeId
                \param newInverseName
                \return  true on success
            */
            bool
            setInverseNameAttribute(NodeId &nodeId,
                                    LocalizedText &newInverseName) {
                return   writeAttribute(nodeId, UA_ATTRIBUTEID_INVERSENAME,
                                        &newInverseName,
                                        &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
            }
            /*!
                \brief setContainsNoLoopsAttribute
                \param nodeId
                \param newContainsNoLoops
                \return  true on success
            */
            bool
            setContainsNoLoopsAttribute(NodeId &nodeId,
                                        UA_Boolean &newContainsNoLoops) {
                return   writeAttribute(nodeId,
                                        UA_ATTRIBUTEID_CONTAINSNOLOOPS,
                                        &newContainsNoLoops,
                                        &UA_TYPES[UA_TYPES_BOOLEAN]);
            }
            /*!
                \brief setEventNotifierAttribute
                \param nodeId
                \param newEventNotifier
                \return  true on success
            */
            bool
            setEventNotifierAttribute(NodeId &nodeId,
                                      UA_Byte newEventNotifier) {
                return   writeAttribute(nodeId,
                                        UA_ATTRIBUTEID_EVENTNOTIFIER,
                                        &newEventNotifier,
                                        &UA_TYPES[UA_TYPES_BYTE]);
            }
            /*!
                \brief setValueAttribute
                \param nodeId
                \param newValue
                \return  true on success
            */
            bool
            setValueAttribute(NodeId &nodeId,
                              Variant &newValue) {
                return   writeAttribute(nodeId, UA_ATTRIBUTEID_VALUE,
                                        newValue, &UA_TYPES[UA_TYPES_VARIANT]);
            }
            /*!
                \brief setDataTypeAttribute
                \param nodeId
                \param newDataType
                \return  true on success
            */
            bool
            setDataTypeAttribute(NodeId &nodeId,
                                 const UA_NodeId *newDataType) {
                return   writeAttribute(nodeId, UA_ATTRIBUTEID_DATATYPE,
                                        newDataType, &UA_TYPES[UA_TYPES_NODEID]);
            }
            /*!
                \brief setValueRankAttribute
                \param nodeId
                \param newValueRank
                \return   true on success
            */
            bool
            setValueRankAttribute(NodeId &nodeId,
                                  UA_Int32 newValueRank) {
                return   writeAttribute(nodeId, UA_ATTRIBUTEID_VALUERANK,
                                        &newValueRank, &UA_TYPES[UA_TYPES_INT32]);
            }
            /*!
                \brief setArrayDimensionsAttribute
                \param nodeId
                \param newArrayDimensions
                \return   true on success
            */
            bool
            setArrayDimensionsAttribute(NodeId &nodeId,
                                        std::vector<UA_UInt32> &newArrayDimensions) {
                UA_UInt32 v = newArrayDimensions.size();
                _lastError = UA_Client_writeArrayDimensionsAttribute(_client, nodeId, v,
                                                                     newArrayDimensions.data());
                return lastOK();
            }
            /*!
                \brief setAccessLevelAttribute
                \param nodeId
                \param newAccessLevel
                \return   true on success
            */
            bool
            setAccessLevelAttribute(NodeId &nodeId,
                                    UA_Byte newAccessLevel) {
                return   writeAttribute(nodeId, UA_ATTRIBUTEID_ACCESSLEVEL,
                                        &newAccessLevel, &UA_TYPES[UA_TYPES_BYTE]);
            }
            /*!
                \brief setUserAccessLevelAttribute
                \param nodeId
                \param newUserAccessLevel
                \return   true on success
            */
            bool
            setUserAccessLevelAttribute(NodeId &nodeId,
                                        UA_Byte newUserAccessLevel) {
                return   writeAttribute(nodeId,
                                        UA_ATTRIBUTEID_USERACCESSLEVEL,
                                        &newUserAccessLevel,
                                        &UA_TYPES[UA_TYPES_BYTE]);
            }
            /*!
                \brief setMinimumSamplingIntervalAttribute
                \param nodeId
                \param newMinInterval
                \return   true on success
            */
            bool
            setMinimumSamplingIntervalAttribute(
                NodeId &nodeId,
                UA_Double newMinInterval) {
                return   writeAttribute(nodeId,
                                        UA_ATTRIBUTEID_MINIMUMSAMPLINGINTERVAL,
                                        &newMinInterval, &UA_TYPES[UA_TYPES_DOUBLE]);
            }
            /*!
                \brief setHistorizingAttribute
                \param nodeId
                \param newHistorizing
                \return   true on success
            */
            bool
            setHistorizingAttribute(NodeId &nodeId,
                                    UA_Boolean newHistorizing) {
                return   writeAttribute(nodeId, UA_ATTRIBUTEID_HISTORIZING,
                                        &newHistorizing, &UA_TYPES[UA_TYPES_BOOLEAN]);
            }
            /*!
                \brief setExecutableAttribute
                \param nodeId
                \param newExecutable
                \return   true on success
            */
            bool
            setExecutableAttribute(NodeId &nodeId,
                                   UA_Boolean newExecutable) {
                _lastError =  writeAttribute(nodeId, UA_ATTRIBUTEID_EXECUTABLE,
                                             &newExecutable, &UA_TYPES[UA_TYPES_BOOLEAN]);
                return lastOK();
            }
            /*!
                \brief setUserExecutableAttribute
                \param nodeId
                \param newUserExecutable
                \return   true on success
            */
            bool
            setUserExecutableAttribute(NodeId &nodeId,
                                       UA_Boolean newUserExecutable) {
                _lastError =  writeAttribute(nodeId,
                                             UA_ATTRIBUTEID_USEREXECUTABLE,
                                             &newUserExecutable,
                                             &UA_TYPES[UA_TYPES_BOOLEAN]);
                return lastOK();
            }

            // End of attributes

            /*!
                \brief variable
                \param nodeId
                \param value
                \return   true on success
            */
            bool  variable(NodeId &nodeId,  Variant &value) {
                WriteLock l(_mutex);
                // outValue is managed by caller - transfer to output value
                value.clear();
                _lastError = UA_Client_readValueAttribute(_client, nodeId, value); // shallow copy
                return lastOK();
            }

            /*!
                \brief nodeClass
                \param nodeId
                \param c
                \return   true on success
            */
            bool nodeClass(NodeId &nodeId, NodeClass &c) {
                WriteLock l(_mutex);
                if (!_client) throw std::runtime_error("Null client");
                _lastError = UA_Client_readNodeClassAttribute(_client, nodeId, &c);
                return lastOK();
            }

            /*!
                \brief deleteNode
                \param nodeId
                \param deleteReferences
                \return   true on success
            */
            bool deleteNode(NodeId &nodeId, bool  deleteReferences) {
                WriteLock l(_mutex);
                if (!_client) throw std::runtime_error("Null client");
                _lastError =  UA_Client_deleteNode(_client, nodeId, UA_Boolean(deleteReferences));
                return lastOK();
            }

            /*!
                \brief deleteTree
                \param nodeId
                \return   true on success
            */
            bool deleteTree(NodeId &nodeId); // recursive delete

            /*!
                \brief Client::deleteChildren
                \param n
            */
            void deleteChildren(UA_NodeId &n);
            /*!
                \brief callMethod
                \param objectId
                \param methodId
                \param in
                \param out
                \return   true on success
            */
            bool callMethod(NodeId &objectId,  NodeId &methodId, VariantList &in, VariantCallResult &out) {
                WriteLock l(_mutex);
                size_t outputSize = 0;
                UA_Variant *output = nullptr;
                if (!_client) throw std::runtime_error("Null client");
                _lastError = UA_STATUSCODE_GOOD;
                _lastError = UA_Client_call(_client,  objectId,
                                            methodId, in.size(), in.data(),
                                            &outputSize, &output);
                if (_lastError == UA_STATUSCODE_GOOD) {
                    out.set(output, outputSize);
                }
                return lastOK();
            }


            /*!
                \brief process
                \return   true on success
            */
            virtual bool process() {
                UA_Client_runAsync(_client, 1000); // drive the async subscriptions
                return true;
            }

            /*!
                \brief lastOK
                \return   true if last error is UA_STATUSCODE_GOOD
            */
            bool lastOK() const {
                return _lastError == UA_STATUSCODE_GOOD;
            }


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
            bool
            addVariableTypeNode(
                NodeId  &requestedNewNodeId,
                NodeId  &parentNodeId,
                NodeId  &referenceTypeId,
                QualifiedName &browseName,
                VariableTypeAttributes &attr,
                NodeId &outNewNodeId = NodeId::Null) {
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
            bool
            addObjectNode(NodeId  &requestedNewNodeId,
                          NodeId  &parentNodeId,
                          NodeId  &referenceTypeId,
                          QualifiedName &browseName,
                          NodeId  &typeDefinition,
                          ObjectAttributes &attr,
                          NodeId &outNewNodeId = NodeId::Null) {
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
            bool
            addObjectTypeNode(NodeId  &requestedNewNodeId,
                              NodeId  &parentNodeId,
                              NodeId  &referenceTypeId,
                              QualifiedName &browseName,
                              ObjectTypeAttributes &attr,
                              NodeId &outNewNodeId = NodeId::Null) {
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
            bool
            addViewNode(NodeId  &requestedNewNodeId,
                        NodeId  &parentNodeId,
                        NodeId  &referenceTypeId,
                        QualifiedName &browseName,
                        ViewAttributes &attr,
                        NodeId &outNewNodeId = NodeId::Null) {
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
            bool
            addReferenceTypeNode(
                NodeId  &requestedNewNodeId,
                NodeId  &parentNodeId,
                NodeId  &referenceTypeId,
                QualifiedName &browseName,
                ReferenceTypeAttributes &attr,
                NodeId &outNewNodeId = NodeId::Null) {
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
            bool
            addDataTypeNode(NodeId  &requestedNewNodeId,
                            NodeId  &parentNodeId,
                            NodeId  &referenceTypeId,
                            QualifiedName &browseName,
                            DataTypeAttributes &attr,
                            NodeId &outNewNodeId = NodeId::Null) {
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
            bool
            addMethodNode(NodeId  &requestedNewNodeId,
                          NodeId  &parentNodeId,
                          NodeId  &referenceTypeId,
                          QualifiedName &browseName,
                          MethodAttributes &attr,
                          NodeId &outNewNodeId = NodeId::Null) {
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
            bool addProperty(NodeId &parent,
                             const std::string &key,
                             Variant &value,
                             NodeId &nodeId,
                             NodeId &newNode = NodeId::Null, int nameSpaceIndex = 0);

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
            static void asyncServiceCallback(UA_Client *client, void *userdata,
                                             UA_UInt32 requestId, void *response,
                                             const UA_DataType *responseType);

            /*!
                \brief asyncService
                \param userdata
                \param requestId
                \param response
                \param responseType
            */
            virtual void asyncService(void * /*userdata*/, UA_UInt32 /*requestId*/, void * /*response*/,
                                      const UA_DataType * /*responseType*/) {}
    };



}

#endif // OPEN62541CLIENT_H
