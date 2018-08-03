/*
    Copyright (C) 2017 -  B. J. Hill

    This file is part of open62541 C++ classes. open62541 C++ classes are free software: you can
    redistribute it and/or modify it under the terms of the Mozilla Public
    License v2.0 as stated in the LICENSE file provided with open62541.

    open62541 C++ classes are distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
    A PARTICULAR PURPOSE.
*/
#ifndef OPEN62541SERVER_H
#define OPEN62541SERVER_H
#include "open62541objects.h"
#include "nodecontext.h"
#include "servermethod.h"
#include "serverrepeatedcallback.h"

namespace Open62541 {


    /*!
        \brief The Server class - this abstracts the server side
    */
    // C++ wrappers for the UA_Server_* calls
    // Be careful often data items are shallow copied not deep copied
    // Check if items have to be deleted or not - get it wrong and you either have crashes or memory leaks
    //
    /*!
        \brief The Server class
    */
    class  UA_EXPORT  Server {
            UA_ServerConfig *_config = nullptr;
            UA_Server *_server; // assume one server per application
            UA_Boolean  _running = false;
            int _port = 4840;
            //
            std::map<std::string, SeverRepeatedCallbackRef> _callbacks;
            //
            ReadWriteMutex _mutex;
            //

            // Lifecycle call backs
            /* Can be NULL. May replace the nodeContext */
            static UA_StatusCode constructor(UA_Server *server,
                                             const UA_NodeId *sessionId, void *sessionContext,
                                             const UA_NodeId *nodeId, void **nodeContext);

            /*  Can be NULL. The context cannot be replaced since the node is destroyed
                immediately afterwards anyway. */
            static void destructor(UA_Server *server,
                                   const UA_NodeId *sessionId, void *sessionContext,
                                   const UA_NodeId *nodeId, void *nodeContext);

            // Map of servers key by UA_Server pointer
            typedef std::map<UA_Server *, Server *> ServerMap;
            static ServerMap _serverMap;

            std::map<UA_UInt64,std::string> _discoveryList; // set of discovery servers this server has registered with

        protected:
            UA_StatusCode _lastError = 0;
        public:
            /*!
                \brief Server
                \param p
            */
            Server(int p = 4840, UA_ByteString *certificate = nullptr )
                : _config(UA_ServerConfig_new_minimal(p,certificate)), _port(p) {
                _config->nodeLifecycle.constructor = constructor; // set up the node global lifecycle
                _config->nodeLifecycle.destructor = destructor;

            }

            virtual ~Server() {
                // possible abnormal exit
                if (_server) {
                   WriteLock l(_mutex);
                   terminate();
                }
                if (_config) UA_ServerConfig_delete(_config);
            }

            /*!
                \brief setServerUri
                \param s
            */
            void setServerUri(const std::string &s) {
                UA_String_deleteMembers(&_config->applicationDescription.applicationUri);
                _config->applicationDescription.applicationUri = UA_String_fromChars(s.c_str());
            }

            /*!
             * \brief setMdnsServerName
             * \param s
             */
            void setMdnsServerName(const std::string &s)
            {
              _config->mdnsServerName = UA_String_fromChars(s.c_str());
            }

            /*!
                \brief findServer
                \param s
                \return
            */
            static Server *findServer(UA_Server *s) {
                return _serverMap[s];
            }
            //
            // Discovery
            //
            /*!
                \brief registerDiscovery
                \param discoveryServerUrl
                \param semaphoreFilePath
                \return
            */
            bool registerDiscovery(const std::string &discoveryServerUrl,  const std::string &semaphoreFilePath = "") {
                _lastError = UA_Server_register_discovery(server(),
                                                          discoveryServerUrl.c_str(),
                                                          (semaphoreFilePath.empty()) ? nullptr : semaphoreFilePath.c_str());
                return lastOK();
            }

            /*!
                \brief unregisterDiscovery
                \return
            */
            bool unregisterDiscovery(const std::string &discoveryServerUrl) {
                _lastError = UA_Server_unregister_discovery(server(), discoveryServerUrl.c_str());
                return lastOK();
            }
            /*!
             * \brief unregisterDiscovery
             * \param periodicCallbackId
             * \return
             */
            bool unregisterDiscovery(UA_UInt64 &periodicCallbackId)
            {
                bool ret = unregisterDiscovery(_discoveryList[periodicCallbackId]);
                _discoveryList.erase(periodicCallbackId);
                return ret;
            }

            /*!
                \brief addPeriodicServerRegister
                \param discoveryServerUrl
                \param intervalMs
                \param delayFirstRegisterMs
                \param periodicCallbackId
                \return
            */
            bool  addPeriodicServerRegister(const std::string &discoveryServerUrl,
                                                    UA_UInt64 &periodicCallbackId,
                                                    UA_UInt32 intervalMs = 600 * 1000, // default to 10 minutes
                                                    UA_UInt32 delayFirstRegisterMs = 1000) {
                _lastError = UA_Server_addPeriodicServerRegisterCallback(server(),
                                                                         discoveryServerUrl.c_str(),
                                                                         intervalMs,
                                                                         delayFirstRegisterMs,
                                                                         &periodicCallbackId);
                //
                if(lastOK())
                {
                  _discoveryList[periodicCallbackId]  = discoveryServerUrl;
                }
                //
                return lastOK();
            }


            /*!
                \brief registerServer
            */
            virtual void registerServer(const UA_RegisteredServer * /*registeredServer*/) {
                OPEN62541_TRC
            }

            /*!
                \brief registerServerCallback
                \param registeredServer
                \param data
            */
            static void registerServerCallback(const UA_RegisteredServer *registeredServer, void *data);
            /*!
                \brief setRegisterServerCallback
            */
            void setRegisterServerCallback() {
                UA_Server_setRegisterServerCallback(server(), registerServerCallback, (void *)(this));
            }

            /*!
                \brief serverOnNetwork
                \param serverOnNetwork
                \param isServerAnnounce
                \param isTxtReceived
            */
            virtual void serverOnNetwork(const UA_ServerOnNetwork * /*serverOnNetwork*/,
                                         UA_Boolean /*isServerAnnounce*/,
                                         UA_Boolean /*isTxtReceived*/) {
                OPEN62541_TRC

            }

            /*!
                \brief serverOnNetworkCallback
                \param serverNetwork
                \param isServerAnnounce
                \param isTxtReceived
                \param data
            */
            static void serverOnNetworkCallback(const UA_ServerOnNetwork *serverNetwork,
                                                UA_Boolean isServerAnnounce,
                                                UA_Boolean isTxtReceived,
                                                void *data);
            #ifdef UA_ENABLE_DISCOVERY_MULTICAST
            /*!
                \brief setServerOnNetworkCallback
            */
            void setServerOnNetworkCallback() {
                UA_Server_setServerOnNetworkCallback(server(), serverOnNetworkCallback, (void *)(this));
            }
            #endif
            /*!
                \brief start
                \param iterate
            */
            virtual void start();  // start the server
            /*!
                \brief stop
            */
            virtual void stop();  // stop the server - do not try start-stop-start
            /*!
                \brief initialise
            */
            virtual void initialise(); // called after the server object has been created but before run has been called
            /*!
                \brief process
            */
            virtual void process() {} // called between server loop iterations - hook thread event processing

            /*!
             * \brief terminate
             */
            virtual void terminate(); // called before server is closed
            //
            /*!
                \brief lastError
                \return
            */
            UA_StatusCode lastError() const {
                return _lastError;
            }
            /*!
                \brief port
                \return
            */
            int port() const {
                return _port;
            }
            /*!
                \brief setPort
                \param p
            */
            void setPort(int p) {
                _port = p;
            }

            /*!
                \brief server
                \return
            */
            UA_Server *server() const {
                return _server;
            }
            /*!
                \brief running
                \return
            */
            UA_Boolean  running() const {
                return _running;
            }


            /*!
                \brief getNodeContext
                \param n
                \param c
                \return
            */
            bool getNodeContext(NodeId &n, NodeContext *&c) {
                void *p = (void *)(c);
                _lastError = UA_Server_getNodeContext(_server, n.get(), &p);
                return lastOK();
            }

            /*!
                \brief findContext
                \return
            */
            static NodeContext *findContext(const std::string &s);

            /* Careful! The user has to ensure that the destructor callbacks still work. */
            /*!
                \brief setNodeContext
                \param n
                \param c
                \return
            */
            bool setNodeContext(NodeId &n, const NodeContext *c) {
                _lastError = UA_Server_setNodeContext(_server, n.get(), (void *)(c));
                return lastOK();
            }


            /*!
                \brief readAttribute
                \param nodeId
                \param attributeId
                \param v
                \return
            */
            bool readAttribute(const UA_NodeId *nodeId, UA_AttributeId attributeId, void *v) {
                WriteLock l(_mutex);
                _lastError =  __UA_Server_read(_server, nodeId, attributeId, v);
                return lastOK();
            }

            /*!
                \brief writeAttribute
                \param nodeId
                \param attributeId
                \param attr_type
                \param attr
                \return
            */
            bool writeAttribute(const UA_NodeId *nodeId, const UA_AttributeId attributeId, const UA_DataType *attr_type, const void *attr) {
                WriteLock l(_mutex);
                _lastError =  __UA_Server_write(_server, nodeId, attributeId, attr_type, attr) == UA_STATUSCODE_GOOD;
                return lastOK();
            }
            /*!
                \brief mutex
                \return
            */
            ReadWriteMutex &mutex() {
                return _mutex; // access mutex - most accesses need a write lock
            }

            /*!
                \brief deleteTree
                \param nodeId
                \return
            */
            bool deleteTree(NodeId &nodeId);
            /*!
                \brief browseTree
                \param nodeId
                \param node
                \return
            */
            bool browseTree(UA_NodeId &nodeId, Open62541::UANode *node); // add child nodes to property tree node

            /*!
                \brief browseTree
                \param nodeId
                \return
            */
            bool browseTree(NodeId &nodeId, UANodeTree &tree); // produces and addressable tree using dot seperated browse path
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
            bool browseTree(NodeId &nodeId, NodeIdMap &m); // browse and create a map of string version of nodeids ids to node ids
            /*!
                \brief browseChildren
                \param nodeId
                \param m
                \return
            */
            bool browseChildren(UA_NodeId &nodeId, NodeIdMap &m);

            /*!
                \brief createBrowsePath
                \param p
                \param tree
                \return
            */
            bool createBrowsePath(NodeId &parent, UAPath &p, UANodeTree &tree); // create a browse path and add it to the tree
            /*!
                \brief addNamespace
                \param s
                \return
            */
            UA_UInt16 addNamespace(const std::string s) {
                UA_UInt16 ret = 0;
                {
                    WriteLock l(mutex());
                    ret =   UA_Server_addNamespace(_server, s.c_str());
                }
                return ret;
            }
            //
            UA_ServerConfig    &serverConfig() {
                return *_config;
            }
            //

            /*!
                \brief addServerMethod
                \param parent
                \param nodeId
                \param newNode
                \param nameSpaceIndex
                \return
            */
            bool addServerMethod(ServerMethod *method, const std::string &browseName,
                                 NodeId &parent,  NodeId &nodeId,
                                 NodeId &newNode,  int nameSpaceIndex = 0) {
                //
                //
                if (nameSpaceIndex == 0) nameSpaceIndex = parent.nameSpaceIndex(); // inherit parent by default
                //
                MethodAttributes attr;
                attr.setDefault();
                attr.setDisplayName(browseName);
                attr.setDescription(browseName);
                attr.setExecutable();
                //
                QualifiedName qn(nameSpaceIndex, browseName);
                {
                    WriteLock l(mutex());
                    _lastError = UA_Server_addMethodNode(_server,
                                                         nodeId,
                                                         parent,
                                                         NodeId::HasOrderedComponent,
                                                         qn,
                                                         attr,
                                                         ServerMethod::methodCallback,
                                                         method->in().size() - 1,
                                                         method->in().data(),
                                                         method->out().size() - 1,
                                                         method->out().data(),
                                                         (void *)(method), // method context is reference to the call handler
                                                         newNode.isNull() ? nullptr : newNode.ref());

                }
                return lastOK();
            }



            /*!
                \brief addRepeatedCallback
                \param id
                \param p
            */
            void addRepeatedCallback(const std::string &id, SeverRepeatedCallback  *p) {
                _callbacks[id] = SeverRepeatedCallbackRef(p);
            }

            /*!
                \brief addRepeatedCallback
                \param id
                \param interval
                \param f
            */
            void addRepeatedCallback(const std::string &id, int interval, SeverRepeatedCallbackFunc f) {
                auto p = new SeverRepeatedCallback(*this, interval, f);
                _callbacks[id] = SeverRepeatedCallbackRef(p);
            }

            /*!
                \brief removeRepeatedCallback
                \param id
            */
            void removeRepeatedCallback(const std::string &id) {
                _callbacks.erase(id);
            }

            /*!
                \brief repeatedCallback
                \param s
                \return
            */
            SeverRepeatedCallbackRef &repeatedCallback(const std::string &s) {
                return _callbacks[s];
            }

            //
            //
            /*!
                \brief browseName
                \param nodeId
                \return
            */
            bool  browseName(NodeId &nodeId, std::string &s, int &ns) {
                if (!_server) throw std::runtime_error("Null server");
                QualifiedName outBrowseName;
                if (UA_Server_readBrowseName(_server, nodeId, outBrowseName) == UA_STATUSCODE_GOOD) {
                    s =   toString(outBrowseName.get().name);
                    ns = outBrowseName.get().namespaceIndex;
                }
                return lastOK();
            }


            /*!
                \brief setBrowseName
                \param nodeId
                \param nameSpaceIndex
                \param name
            */
            void setBrowseName(NodeId &nodeId, int nameSpaceIndex, const std::string &name) {
                QualifiedName newBrowseName(nameSpaceIndex, name);
                WriteLock l(_mutex);
                UA_Server_writeBrowseName(_server, nodeId, newBrowseName);
            }


            /*!
                \brief NodeIdFromPath get the node id from the path of browse names in the given namespace. Tests for node existance
                \param path
                \param nodeId
                \return
            */
            bool nodeIdFromPath(NodeId &start, Path &path,  NodeId &nodeId);

            /*!
                \brief createPath
                \param start
                \param path
                \param nameSpaceIndex
                \param nodeId
                \return
            */
            bool createFolderPath(NodeId &start, Path &path, int nameSpaceIndex, NodeId &nodeId);

            /*!
                \brief getChild
                \param nameSpaceIndex
                \param childName
                \return
            */
            bool  getChild(NodeId &start, const std::string &childName, NodeId &ret);



            /*!
                \brief addFolder
                \param parent
                \param nameSpaceIndex
                \param childName
                \return
            */
            bool addFolder(NodeId &parent,  const std::string &childName,
                           NodeId &nodeId, NodeId &newNode = NodeId::Null, int nameSpaceIndex = 0);

            /*!
                \brief addVariable
                \param parent
                \param nameSpaceIndex
                \param childName
                \return
            */
            bool addVariable(NodeId &parent, const std::string &childName,
                             Variant &value, NodeId &nodeId,  NodeId &newNode = NodeId::Null,
                             NodeContext *c = nullptr,
                             int nameSpaceIndex = 0);

            template<typename T>
            /*!
                \brief addVariable
                \param parent
                \param childName
                \param nodeId
                \param c
                \param newNode
                \param nameSpaceIndex
                \return
            */
            bool addVariable(NodeId &parent,  const std::string &childName,
                             NodeId &nodeId, const std::string &c,
                             NodeId &newNode = NodeId::Null,
                             int nameSpaceIndex = 0) {
                NodeContext *cp = findContext(c);
                if (cp) {
                    Variant v(T());
                    return  addVariable(parent, childName, v, nodeId,  newNode, cp, nameSpaceIndex);
                }
                return false;
            }

            template <typename T>
            /*!
                \brief addProperty
                \param parent
                \param key
                \param value
                \param nodeId
                \param newNode
                \param c
                \param nameSpaceIndex
                \return
            */
            bool addProperty(NodeId &parent,
                             const std::string &key,
                             const T &value,
                             NodeId &nodeId  = NodeId::Null,
                             NodeId &newNode = NodeId::Null,
                             NodeContext *c = nullptr,
                             int nameSpaceIndex = 0) {
                Variant v(value);
                return addProperty(parent, key, v, nodeId, newNode, c, nameSpaceIndex);
            }

            /*!
                \brief addProperty
                \param parent
                \param key
                \param value
                \param nodeId
                \param newNode
                \param c
                \param nameSpaceIndex
                \return
            */
            bool addProperty(NodeId &parent,
                             const std::string &key,
                             Variant &value,
                             NodeId &nodeId  = NodeId::Null,
                             NodeId &newNode = NodeId::Null,
                             NodeContext *c = nullptr,
                             int nameSpaceIndex = 0);

            /*!
                \brief variable
                \param nodeId
                \param value
                \return
            */
            bool  variable(NodeId &nodeId,  Variant &value) {
                // outValue is managed by caller - transfer to output value
                value.null();
                WriteLock l(_mutex);
                UA_Server_readValue(_server, nodeId, value.ref());
                return lastOK();
            }
            /*!
                \brief deleteNode
                \param nodeId
                \param deleteReferences
                \return
            */
            bool deleteNode(NodeId &nodeId, bool  deleteReferences) {
                WriteLock l(_mutex);
                _lastError =  UA_Server_deleteNode(_server, nodeId, UA_Boolean(deleteReferences));
                return _lastError != UA_STATUSCODE_GOOD;
            }

            /*!
                \brief call
                \param request
                \param ret
                \return
            */
            bool call(CallMethodRequest &request, CallMethodResult &ret) {
                WriteLock l(_mutex);
                ret.get() = UA_Server_call(_server, request);
                return ret.get().statusCode == UA_STATUSCODE_GOOD;
            }

            /*!
                \brief translateBrowsePathToNodeIds
                \param path
                \param result
                \return
            */
            bool translateBrowsePathToNodeIds(BrowsePath &path, BrowsePathResult &result) {
                WriteLock l(_mutex);
                result.get() = UA_Server_translateBrowsePathToNodeIds(_server, path);
                return result.get().statusCode  == UA_STATUSCODE_GOOD;
            }

            /*!
                \brief lastOK
                \return
            */
            bool lastOK() const {
                return _lastError == UA_STATUSCODE_GOOD;
            }
            //
            // Attributes
            //
            /*!
                \brief readNodeId
                \param nodeId
                \param outNodeId
                \return
            */
            bool
            readNodeId(NodeId &nodeId,
                       NodeId &outNodeId) {
                return  readAttribute(nodeId, UA_ATTRIBUTEID_NODEID, outNodeId);
            }
            /*!
                \brief readNodeClass
                \param nodeId
                \param outNodeClass
                \return
            */
            bool
            readNodeClass(NodeId &nodeId,
                          UA_NodeClass &outNodeClass) {
                return  readAttribute(nodeId, UA_ATTRIBUTEID_NODECLASS,
                                      &outNodeClass);
            }
            /*!
                \brief readBrowseName
                \param nodeId
                \param outBrowseName
                \return
            */
            bool
            readBrowseName(NodeId &nodeId,
                           QualifiedName &outBrowseName) {
                return  readAttribute(nodeId, UA_ATTRIBUTEID_BROWSENAME,
                                      outBrowseName);
            }
            /*!
                \brief readDisplayName
                \param nodeId
                \param outDisplayName
                \return
            */
            bool
            readDisplayName(NodeId &nodeId,
                            LocalizedText &outDisplayName) {
                return  readAttribute(nodeId, UA_ATTRIBUTEID_DISPLAYNAME,
                                      outDisplayName);
            }
            /*!
                \brief readDescription
                \param nodeId
                \param outDescription
                \return
            */
            bool
            readDescription(NodeId &nodeId,
                            LocalizedText &outDescription) {
                return  readAttribute(nodeId, UA_ATTRIBUTEID_DESCRIPTION,
                                      outDescription);
            }
            /*!
                \brief readWriteMask
                \param nodeId
                \param outWriteMask
                \return
            */
            bool
            readWriteMask(NodeId &nodeId,
                          UA_UInt32 &outWriteMask) {
                return  readAttribute(nodeId, UA_ATTRIBUTEID_WRITEMASK,
                                      &outWriteMask);
            }
            /*!
                \brief readIsAbstract
                \param nodeId
                \param outIsAbstract
                \return
            */
            bool
            readIsAbstract(NodeId &nodeId,
                           UA_Boolean &outIsAbstract) {
                return  readAttribute(nodeId, UA_ATTRIBUTEID_ISABSTRACT,
                                      &outIsAbstract);
            }
            /*!
                \brief readSymmetric
                \param nodeId
                \param outSymmetric
                \return
            */
            bool
            readSymmetric(NodeId &nodeId,
                          UA_Boolean &outSymmetric) {
                return  readAttribute(nodeId, UA_ATTRIBUTEID_SYMMETRIC,
                                      &outSymmetric);
            }
            /*!
                \brief readInverseName
                \param nodeId
                \param outInverseName
                \return
            */
            bool
            readInverseName(NodeId &nodeId,
                            LocalizedText &outInverseName) {
                return  readAttribute(nodeId, UA_ATTRIBUTEID_INVERSENAME,
                                      outInverseName);
            }
            /*!
                \brief readContainsNoLoop
                \param nodeId
                \param outContainsNoLoops
                \return
            */
            bool
            readContainsNoLoop(NodeId &nodeId,
                               UA_Boolean &outContainsNoLoops) {
                return  readAttribute(nodeId, UA_ATTRIBUTEID_CONTAINSNOLOOPS,
                                      &outContainsNoLoops);
            }
            /*!
                \brief readEventNotifier
                \param nodeId
                \param outEventNotifier
                \return
            */
            bool
            readEventNotifier(NodeId &nodeId,
                              UA_Byte &outEventNotifier) {
                return  readAttribute(nodeId, UA_ATTRIBUTEID_EVENTNOTIFIER,
                                      &outEventNotifier);
            }
            /*!
                \brief readValue
                \param nodeId
                \param outValue
                \return
            */
            bool
            readValue(NodeId &nodeId,
                      Variant &outValue) {
                return  readAttribute(nodeId, UA_ATTRIBUTEID_VALUE, outValue);
            }
            /*!
                \brief readDataType
                \param nodeId
                \param outDataType
                \return
            */
            bool
            readDataType(NodeId &nodeId,
                         NodeId &outDataType) {
                return  readAttribute(nodeId, UA_ATTRIBUTEID_DATATYPE,
                                      outDataType);
            }
            /*!
                \brief readValueRank
                \param nodeId
                \param outValueRank
                \return
            */
            bool
            readValueRank(NodeId &nodeId,
                          UA_Int32 &outValueRank) {
                return  readAttribute(nodeId, UA_ATTRIBUTEID_VALUERANK,
                                      &outValueRank);
            }

            /* Returns a variant with an int32 array */
            /*!
                \brief readArrayDimensions
                \param nodeId
                \param outArrayDimensions
                \return
            */
            bool
            readArrayDimensions(NodeId &nodeId,
                                Variant &outArrayDimensions) {
                return  readAttribute(nodeId, UA_ATTRIBUTEID_ARRAYDIMENSIONS,
                                      outArrayDimensions);
            }
            /*!
                \brief readAccessLevel
                \param nodeId
                \param outAccessLevel
                \return
            */
            bool
            readAccessLevel(NodeId &nodeId,
                            UA_Byte &outAccessLevel) {
                return  readAttribute(nodeId, UA_ATTRIBUTEID_ACCESSLEVEL,
                                      &outAccessLevel);
            }
            /*!
                \brief readMinimumSamplingInterval
                \param nodeId
                \param outMinimumSamplingInterval
                \return
            */
            bool
            readMinimumSamplingInterval(NodeId &nodeId,
                                        UA_Double &outMinimumSamplingInterval) {
                return  readAttribute(nodeId,
                                      UA_ATTRIBUTEID_MINIMUMSAMPLINGINTERVAL,
                                      &outMinimumSamplingInterval);
            }
            /*!
                \brief readHistorizing
                \param nodeId
                \param outHistorizing
                \return
            */
            bool
            readHistorizing(NodeId &nodeId,
                            UA_Boolean &outHistorizing) {
                return  readAttribute(nodeId, UA_ATTRIBUTEID_HISTORIZING,
                                      &outHistorizing);
            }
            /*!
                \brief readExecutable
                \param nodeId
                \param outExecutable
                \return
            */
            bool
            readExecutable(NodeId &nodeId,
                           UA_Boolean &outExecutable) {
                return  readAttribute(nodeId, UA_ATTRIBUTEID_EXECUTABLE,
                                      &outExecutable);
            }
            /*!
                \brief writeBrowseName
                \param nodeId
                \param browseName
                \return
            */
            bool
            writeBrowseName(NodeId &nodeId,
                            QualifiedName &browseName) {
                return  writeAttribute(nodeId, UA_ATTRIBUTEID_BROWSENAME,
                                       &UA_TYPES[UA_TYPES_QUALIFIEDNAME], browseName);
            }
            /*!
                \brief writeDisplayName
                \param nodeId
                \param displayName
                \return
            */
            bool
            writeDisplayName(NodeId &nodeId,
                             LocalizedText &displayName) {
                return  writeAttribute(nodeId, UA_ATTRIBUTEID_DISPLAYNAME,
                                       &UA_TYPES[UA_TYPES_LOCALIZEDTEXT], displayName);
            }
            /*!
                \brief writeDescription
                \param nodeId
                \param description
                \return
            */
            bool
            writeDescription(NodeId &nodeId,
                             LocalizedText &description) {
                return  writeAttribute(nodeId, UA_ATTRIBUTEID_DESCRIPTION,
                                       &UA_TYPES[UA_TYPES_LOCALIZEDTEXT], description);
            }
            /*!
                \brief writeWriteMask
                \param nodeId
                \param writeMask
                \return
            */
            bool
            writeWriteMask(NodeId &nodeId,
                           const UA_UInt32 writeMask) {
                return  writeAttribute(nodeId, UA_ATTRIBUTEID_WRITEMASK,
                                       &UA_TYPES[UA_TYPES_UINT32], &writeMask);
            }
            /*!
                \brief writeIsAbstract
                \param nodeId
                \param isAbstract
                \return
            */
            bool
            writeIsAbstract(NodeId &nodeId,
                            const UA_Boolean isAbstract) {
                return  writeAttribute(nodeId, UA_ATTRIBUTEID_ISABSTRACT,
                                       &UA_TYPES[UA_TYPES_BOOLEAN], &isAbstract);
            }
            /*!
                \brief writeInverseName
                \param nodeId
                \param inverseName
                \return
            */
            bool
            writeInverseName(NodeId &nodeId,
                             const UA_LocalizedText inverseName) {
                return  writeAttribute(nodeId, UA_ATTRIBUTEID_INVERSENAME,
                                       &UA_TYPES[UA_TYPES_LOCALIZEDTEXT], &inverseName);
            }
            /*!
                \brief writeEventNotifier
                \param nodeId
                \param eventNotifier
                \return
            */
            bool
            writeEventNotifier(NodeId &nodeId,
                               const UA_Byte eventNotifier) {
                return  writeAttribute(nodeId, UA_ATTRIBUTEID_EVENTNOTIFIER,
                                       &UA_TYPES[UA_TYPES_BYTE], &eventNotifier);
            }
            /*!
                \brief writeValue
                \param nodeId
                \param value
                \return
            */
            bool
            writeValue(NodeId &nodeId,
                       Variant &value) {
                return  UA_STATUSCODE_GOOD == (_lastError = __UA_Server_write(_server, nodeId, UA_ATTRIBUTEID_VALUE,
                                                                              &UA_TYPES[UA_TYPES_VARIANT], value));
            }
            /*!
                \brief writeDataType
                \param nodeId
                \param dataType
                \return
            */
            bool
            writeDataType(NodeId &nodeId,
                          NodeId &dataType) {
                return  writeAttribute(nodeId, UA_ATTRIBUTEID_DATATYPE,
                                       &UA_TYPES[UA_TYPES_NODEID], dataType);
            }
            /*!
                \brief writeValueRank
                \param nodeId
                \param valueRank
                \return
            */
            bool
            writeValueRank(NodeId &nodeId,
                           const UA_Int32 valueRank) {
                return  writeAttribute(nodeId, UA_ATTRIBUTEID_VALUERANK,
                                       &UA_TYPES[UA_TYPES_INT32], &valueRank);
            }

            /*!
                \brief writeArrayDimensions
                \param nodeId
                \param arrayDimensions
                \return
            */
            bool
            writeArrayDimensions(NodeId &nodeId,
                                 Variant arrayDimensions) {
                return  writeAttribute(nodeId, UA_ATTRIBUTEID_VALUE,
                                       &UA_TYPES[UA_TYPES_VARIANT], arrayDimensions.constRef());
            }
            /*!
                \brief writeAccessLevel
                \param nodeId
                \param accessLevel
                \return
            */
            bool
            writeAccessLevel(NodeId &nodeId,
                             const UA_Byte accessLevel) {
                return  writeAttribute(nodeId, UA_ATTRIBUTEID_ACCESSLEVEL,
                                       &UA_TYPES[UA_TYPES_BYTE], &accessLevel);
            }
            /*!
                \brief writeMinimumSamplingInterval
                \param nodeId
                \param miniumSamplingInterval
                \return
            */
            bool
            writeMinimumSamplingInterval(NodeId &nodeId,
                                         const UA_Double miniumSamplingInterval) {
                return  writeAttribute(nodeId,
                                       UA_ATTRIBUTEID_MINIMUMSAMPLINGINTERVAL,
                                       &UA_TYPES[UA_TYPES_DOUBLE],
                                       &miniumSamplingInterval);
            }
            /*!
                \brief writeExecutable
                \param nodeId
                \param executable
                \return
            */
            bool
            writeExecutable(NodeId &nodeId,
                            const UA_Boolean executable) {
                return  writeAttribute(nodeId, UA_ATTRIBUTEID_EXECUTABLE,
                                       &UA_TYPES[UA_TYPES_BOOLEAN], &executable);
            }

            //
            // Add Nodes - taken from docs
            //
            /*!
                \brief addVariableNode
                \param requestedNewNodeId
                \param parentNodeId
                \param referenceTypeId
                \param browseName
                \param typeDefinition
                \param attr
                \param outNewNodeId
                \param instantiationCallback
                \return
            */
            bool
            addVariableNode(NodeId &requestedNewNodeId,
                            NodeId &parentNodeId,
                            NodeId &referenceTypeId,
                            QualifiedName &browseName,
                            NodeId &typeDefinition,
                            VariableAttributes &attr,
                            NodeId &outNewNodeId = NodeId::Null,
                            NodeContext *nc = nullptr) {
                WriteLock l(_mutex);
                _lastError = UA_Server_addVariableNode(_server,
                                                       requestedNewNodeId,
                                                       parentNodeId,
                                                       referenceTypeId,
                                                       browseName,
                                                       typeDefinition,
                                                       attr,
                                                       nc,
                                                       outNewNodeId.isNull() ? nullptr : outNewNodeId.ref());

                return lastOK();
            }

            /*!
                \brief addVariableTypeNode
                \param requestedNewNodeId
                \param parentNodeId
                \param referenceTypeId
                \param browseName
                \param typeDefinition
                \param attr
                \param outNewNodeId
                \param instantiationCallback
                \return
            */
            bool
            addVariableTypeNode(
                NodeId &requestedNewNodeId,
                NodeId &parentNodeId,
                NodeId &referenceTypeId,
                QualifiedName &browseName,
                NodeId &typeDefinition,
                VariableTypeAttributes &attr,
                NodeId &outNewNodeId = NodeId::Null,
                NodeContext *instantiationCallback = nullptr) {
                WriteLock l(_mutex);
                _lastError =  UA_Server_addVariableTypeNode(_server,
                                                            requestedNewNodeId,
                                                            parentNodeId,
                                                            referenceTypeId,
                                                            browseName,
                                                            typeDefinition,
                                                            attr,
                                                            instantiationCallback,
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
                \param instantiationCallback
                \return
            */
            bool
            addObjectNode(NodeId &requestedNewNodeId,
                          NodeId &parentNodeId,
                          NodeId &referenceTypeId,
                          QualifiedName &browseName,
                          NodeId &typeDefinition,
                          ObjectAttributes &attr,
                          NodeId &outNewNodeId = NodeId::Null,
                          NodeContext *instantiationCallback = nullptr) {
                WriteLock l(_mutex);
                _lastError = UA_Server_addObjectNode(_server,
                                                     requestedNewNodeId,
                                                     parentNodeId,
                                                     referenceTypeId,
                                                     browseName,
                                                     typeDefinition,
                                                     attr,
                                                     instantiationCallback,
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
                \param instantiationCallback
                \return
            */
            bool
            addObjectTypeNode(NodeId &requestedNewNodeId,
                              NodeId &parentNodeId,
                              NodeId &referenceTypeId,
                              QualifiedName &browseName,
                              ObjectTypeAttributes &attr,
                              NodeId &outNewNodeId = NodeId::Null,
                              NodeContext *instantiationCallback = nullptr) {
                _lastError = UA_Server_addObjectTypeNode(_server,
                                                         requestedNewNodeId,
                                                         parentNodeId,
                                                         referenceTypeId,
                                                         browseName,
                                                         attr,
                                                         instantiationCallback,
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
                \param instantiationCallback
                \return
            */
            bool
            addViewNode(NodeId &requestedNewNodeId,
                        NodeId &parentNodeId,
                        NodeId &referenceTypeId,
                        QualifiedName &browseName,
                        ViewAttributes &attr,
                        NodeId &outNewNodeId = NodeId::Null,
                        NodeContext *instantiationCallback = nullptr
                       ) {
                WriteLock l(_mutex);
                _lastError = UA_Server_addViewNode(_server,
                                                   requestedNewNodeId,
                                                   parentNodeId,
                                                   referenceTypeId,
                                                   browseName,
                                                   attr,
                                                   instantiationCallback,
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
                \param instantiationCallback
                \return
            */
            bool
            addReferenceTypeNode(
                NodeId &requestedNewNodeId,
                NodeId &parentNodeId,
                NodeId &referenceTypeId,
                QualifiedName &browseName,
                ReferenceTypeAttributes &attr,
                NodeId &outNewNodeId = NodeId::Null,
                NodeContext *instantiationCallback = nullptr
            ) {
                WriteLock l(_mutex);
                _lastError = UA_Server_addReferenceTypeNode(_server,
                                                            requestedNewNodeId,
                                                            parentNodeId,
                                                            referenceTypeId,
                                                            browseName,
                                                            attr,
                                                            instantiationCallback,
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
                \param instantiationCallback
                \return
            */
            bool
            addDataTypeNode(
                NodeId &requestedNewNodeId,
                NodeId &parentNodeId,
                NodeId &referenceTypeId,
                QualifiedName &browseName,
                DataTypeAttributes &attr,
                NodeId &outNewNodeId = NodeId::Null,
                NodeContext *instantiationCallback = nullptr
            ) {
                WriteLock l(_mutex);
                _lastError = UA_Server_addDataTypeNode(_server,
                                                       requestedNewNodeId,
                                                       parentNodeId,
                                                       referenceTypeId,
                                                       browseName,
                                                       attr,
                                                       instantiationCallback,
                                                       outNewNodeId.isNull() ? nullptr : outNewNodeId.ref());
                return lastOK();
            }

            /*!
                \brief addDataSourceVariableNode
                \param requestedNewNodeId
                \param parentNodeId
                \param referenceTypeId
                \param browseName
                \param typeDefinition
                \param attr
                \param dataSource
                \param outNewNodeId
                \return
            */
            bool
            addDataSourceVariableNode(
                NodeId &requestedNewNodeId,
                NodeId &parentNodeId,
                NodeId &referenceTypeId,
                QualifiedName &browseName,
                NodeId &typeDefinition,
                VariableAttributes &attr,
                DataSource &dataSource,
                NodeId &outNewNodeId = NodeId::Null,
                NodeContext *instantiationCallback = nullptr
            ) {
                WriteLock l(_mutex);
                _lastError = UA_Server_addDataSourceVariableNode(_server,
                                                                 requestedNewNodeId,
                                                                 parentNodeId,
                                                                 referenceTypeId,
                                                                 browseName,
                                                                 typeDefinition,
                                                                 attr,
                                                                 dataSource,
                                                                 instantiationCallback,
                                                                 outNewNodeId.isNull() ? nullptr : outNewNodeId.ref());

                return lastOK();

            }

            /*!
                \brief addReference
                \param sourceId
                \param refTypeId
                \param targetId
                \param isForward
                \return
            */
            bool addReference(NodeId &sourceId, NodeId &refTypeId, ExpandedNodeId &targetId, bool isForward) {
                WriteLock l(_mutex);
                _lastError =  UA_Server_addReference(server(), sourceId, refTypeId, targetId, isForward);
                return lastOK();
            }

            /*!
                \brief markMandatory
                \param nodeId
                \return
            */
            bool markMandatory(NodeId &nodeId) {
                return addReference(nodeId, NodeId::HasModellingRule, ExpandedNodeId::ModellingRuleMandatory, true);
            }

            /*!
                \brief deleteReference
                \param sourceNodeId
                \param referenceTypeId
                \param isForward
                \param targetNodeId
                \param deleteBidirectional
                \return
            */
            bool deleteReference(NodeId &sourceNodeId,
                                 NodeId &referenceTypeId, bool isForward,
                                 ExpandedNodeId &targetNodeId,
                                 bool deleteBidirectional) {
                WriteLock l(_mutex);
                _lastError =  UA_Server_deleteReference(server(), sourceNodeId, referenceTypeId,
                                                        isForward, targetNodeId, deleteBidirectional);
                return lastOK();

            }


            /*!
                \brief Open62541::Server::addInstance
                \param n
                \param parent
                \param nodeId
                \return
            */
            bool addInstance(const std::string &n, NodeId &requestedNewNodeId, NodeId &parent,
                             NodeId &typeId, NodeId &nodeId = NodeId::Null, NodeContext *context = nullptr) {
                ObjectAttributes oAttr;
                oAttr.setDefault();
                oAttr.setDisplayName(n);
                QualifiedName qn(parent.nameSpaceIndex(), n);
                return addObjectNode(requestedNewNodeId,
                                     parent,
                                     NodeId::Organizes,
                                     qn,
                                     typeId,
                                     oAttr,
                                     nodeId,
                                     context);
            }


    };



}
#endif // OPEN62541SERVER_H
