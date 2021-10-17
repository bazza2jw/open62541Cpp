/*
    Copyright (C) 2017 -  B. J. Hill

    This file is part of open62541 C++ classes. open62541 C++ classes are free software: you can
    redistribute it and/or modify it under the terms of the Mozilla Public
    License v2.0 as stated in the LICENSE file provided with open62541.

    open62541 C++ classes are distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
    A PARTICULAR PURPOSE.
*/
#include <open62541cpp/open62541server.h>
#include <open62541cpp/nodecontext.h>
#include <open62541cpp/serverbrowser.h>
#include <open62541cpp/open62541client.h>
#include <open62541cpp/historydatabase.h>

// map UA_SERVER to Server objects
Open62541::Server::ServerMap Open62541::Server::_serverMap;

/*!
    \brief Open62541::Server::findContext
    \param s
    \return
*/
Open62541::NodeContext* Open62541::Server::findContext(const std::string& s)
{
    return RegisteredNodeContext::findRef(s);  // not all node contexts are registered
}

/*!
    \brief Open62541::Server::setHistoryDatabase
    \param h
*/
void Open62541::Server::setHistoryDatabase(UA_HistoryDatabase& h)
{
    if (_config)
        _config->historyDatabase = h;
}

// Lifecycle call backs
/* Can be NULL. May replace the nodeContext */
/*!
    \brief Open62541::Server::constructor
    \param server
    \param sessionId
    \param sessionContext
    \param nodeId
    \param nodeContext
    \return
*/
UA_StatusCode Open62541::Server::constructor(UA_Server* server,
                                             const UA_NodeId* /*sessionId*/,
                                             void* /*sessionContext*/,
                                             const UA_NodeId* nodeId,
                                             void** nodeContext)
{
    UA_StatusCode ret = UA_STATUSCODE_GOOD;
    if (server && nodeId && nodeContext) {
        void* p         = *nodeContext;
        NodeContext* cp = (NodeContext*)(p);
        if (cp) {
            Server* s = Server::findServer(server);
            if (s) {
                NodeId n(*nodeId);
                ret = (cp->construct(*s, n)) ? UA_STATUSCODE_GOOD : UA_STATUSCODE_BADINTERNALERROR;
            }
        }
    }
    return ret;
}

/*  Can be NULL. The context cannot be replaced since the node is destroyed
    immediately afterwards anyway. */
/*!
    ! * \brief Open62541::Server::destructor
    ! * \param server
    ! * \param nodeId
    ! * \param nodeContext
    ! */
void Open62541::Server::destructor(UA_Server* server,
                                   const UA_NodeId* /*sessionId*/,
                                   void* /*sessionContext*/,
                                   const UA_NodeId* nodeId,
                                   void* nodeContext)
{
    if (server && nodeId && nodeContext) {
        NodeContext* cp = (NodeContext*)(nodeContext);
        Server* s       = Server::findServer(server);
        if (s) {
            NodeId n(*nodeId);
            cp->destruct(*s, n);
        }
    }
}

/*!
 * \brief Open62541::Server::asyncOperationNotifyCallback
 * \param server
 */
void Open62541::Server::asyncOperationNotifyCallback(UA_Server* server)
{
    Server* p = Open62541::Server::findServer(server);  // find the server
    if (p) {
        p->asyncOperationNotify();
    }
}

void Open62541::Server::monitoredItemRegisterCallback(UA_Server* server,
                                                      const UA_NodeId* sessionId,
                                                      void* sessionContext,
                                                      const UA_NodeId* nodeId,
                                                      void* nodeContext,
                                                      UA_UInt32 attibuteId,
                                                      UA_Boolean removed)
{
    Server* p = Open62541::Server::findServer(server);  // find the server
    if (p) {
        p->monitoredItemRegister(sessionId, sessionContext, nodeId, nodeContext, (uint32_t)attibuteId, (bool)removed);
    }
}

UA_Boolean Open62541::Server::createOptionalChildCallback(UA_Server* server,
                                                          const UA_NodeId* sessionId,
                                                          void* sessionContext,
                                                          const UA_NodeId* sourceNodeId,
                                                          const UA_NodeId* targetParentNodeId,
                                                          const UA_NodeId* referenceTypeId)
{
    Server* p = Open62541::Server::findServer(server);  // find the server
    if (p) {
        return p->createOptionalChild(sessionId, sessionContext, sourceNodeId, targetParentNodeId, referenceTypeId);
    }
    return UA_FALSE;
}

UA_StatusCode Open62541::Server::generateChildNodeIdCallback(UA_Server* server,
                                                             const UA_NodeId* sessionId,
                                                             void* sessionContext,
                                                             const UA_NodeId* sourceNodeId,
                                                             const UA_NodeId* targetParentNodeId,
                                                             const UA_NodeId* referenceTypeId,
                                                             UA_NodeId* targetNodeId)
{
    Server* p = Open62541::Server::findServer(server);  // find the server
    if (p) {
        p->generateChildNodeId(sessionId,
                               sessionContext,
                               sourceNodeId,
                               targetParentNodeId,
                               referenceTypeId,
                               targetNodeId);
    }
    return UA_STATUSCODE_BADSERVERNOTCONNECTED;
}

// Access Control Callbacks
UA_Boolean Open62541::Server::allowAddNodeHandler(UA_Server* server,
                                                  UA_AccessControl* ac,
                                                  const UA_NodeId* sessionId,
                                                  void* sessionContext,
                                                  const UA_AddNodesItem* item)
{
    Server* p = Open62541::Server::findServer(server);  // find the server
    if (p) {
        return p->allowAddNode(ac, sessionId, sessionContext, item);
    }
    return UA_FALSE;
}

UA_Boolean Open62541::Server::allowAddReferenceHandler(UA_Server* server,
                                                       UA_AccessControl* ac,
                                                       const UA_NodeId* sessionId,
                                                       void* sessionContext,
                                                       const UA_AddReferencesItem* item)
{
    Server* p = Open62541::Server::findServer(server);
    if (p) {
        return p->allowAddReference(ac, sessionId, sessionContext, item);
    }
    return UA_FALSE;
}

UA_Boolean Open62541::Server::allowDeleteNodeHandler(UA_Server* server,
                                                     UA_AccessControl* ac,
                                                     const UA_NodeId* sessionId,
                                                     void* sessionContext,
                                                     const UA_DeleteNodesItem* item)
{
    Server* p = Open62541::Server::findServer(server);
    if (p) {
        return p->allowDeleteNode(ac, sessionId, sessionContext, item);
    }

    return UA_FALSE;  // Do not allow deletion from client
}

UA_Boolean Open62541::Server::allowDeleteReferenceHandler(UA_Server* server,
                                                          UA_AccessControl* ac,
                                                          const UA_NodeId* sessionId,
                                                          void* sessionContext,
                                                          const UA_DeleteReferencesItem* item)
{
    Server* p = Open62541::Server::findServer(server);
    if (p) {
        return p->allowDeleteReference(ac, sessionId, sessionContext, item);
    }
    return UA_FALSE;
}

UA_StatusCode Open62541::Server::activateSessionHandler(UA_Server* server,
                                                        UA_AccessControl* ac,
                                                        const UA_EndpointDescription* endpointDescription,
                                                        const UA_ByteString* secureChannelRemoteCertificate,
                                                        const UA_NodeId* sessionId,
                                                        const UA_ExtensionObject* userIdentityToken,
                                                        void** sessionContext)
{
    Server* p = Open62541::Server::findServer(server);
    if (p) {
        return p->activateSession(ac,
                                  endpointDescription,
                                  secureChannelRemoteCertificate,
                                  sessionId,
                                  userIdentityToken,
                                  sessionContext);
    }
    return -1;
}

/* Deauthenticate a session and cleanup */
void Open62541::Server::closeSessionHandler(UA_Server* server,
                                            UA_AccessControl* ac,
                                            const UA_NodeId* sessionId,
                                            void* sessionContext)
{
    Server* p = Open62541::Server::findServer(server);
    if (p) {
        p->closeSession(ac, sessionId, sessionContext);
    }
}

/* Access control for all nodes*/
UA_UInt32 Open62541::Server::getUserRightsMaskHandler(UA_Server* server,
                                                      UA_AccessControl* ac,
                                                      const UA_NodeId* sessionId,
                                                      void* sessionContext,
                                                      const UA_NodeId* nodeId,
                                                      void* nodeContext)
{
    Server* p = Open62541::Server::findServer(server);
    if (p) {
        return p->getUserRightsMask(ac, sessionId, sessionContext, nodeId, nodeContext);
    }
    return 0;
}

/* Additional access control for variable nodes */
UA_Byte Open62541::Server::getUserAccessLevelHandler(UA_Server* server,
                                                     UA_AccessControl* ac,
                                                     const UA_NodeId* sessionId,
                                                     void* sessionContext,
                                                     const UA_NodeId* nodeId,
                                                     void* nodeContext)
{
    Server* p = Open62541::Server::findServer(server);
    if (p) {
        return p->getUserAccessLevel(ac, sessionId, sessionContext, nodeId, nodeContext);
    }
    return 0;
}

/* Additional access control for method nodes */
UA_Boolean Open62541::Server::getUserExecutableHandler(UA_Server* server,
                                                       UA_AccessControl* ac,
                                                       const UA_NodeId* sessionId,
                                                       void* sessionContext,
                                                       const UA_NodeId* methodId,
                                                       void* methodContext)
{
    Server* p = Open62541::Server::findServer(server);
    if (p) {
        return p->getUserExecutable(ac, sessionId, sessionContext, methodId, methodContext);
    }
    return UA_FALSE;
}

/*  Additional access control for calling a method node in the context of a
    specific object */
UA_Boolean Open62541::Server::getUserExecutableOnObjectHandler(UA_Server* server,
                                                               UA_AccessControl* ac,
                                                               const UA_NodeId* sessionId,
                                                               void* sessionContext,
                                                               const UA_NodeId* methodId,
                                                               void* methodContext,
                                                               const UA_NodeId* objectId,
                                                               void* objectContext)
{
    Server* p = Open62541::Server::findServer(server);
    if (p) {
        return p->getUserExecutableOnObject(ac,
                                            sessionId,
                                            sessionContext,
                                            methodId,
                                            methodContext,
                                            objectId,
                                            objectContext);
    }
    return UA_FALSE;
}
/* Allow insert,replace,update of historical data */
UA_Boolean Open62541::Server::allowHistoryUpdateUpdateDataHandler(UA_Server* server,
                                                                  UA_AccessControl* ac,
                                                                  const UA_NodeId* sessionId,
                                                                  void* sessionContext,
                                                                  const UA_NodeId* nodeId,
                                                                  UA_PerformUpdateType performInsertReplace,
                                                                  const UA_DataValue* value)
{
    Server* p = Open62541::Server::findServer(server);
    if (p) {
        return p->allowHistoryUpdateUpdateData(ac, sessionId, sessionContext, nodeId, performInsertReplace, value);
    }
    return UA_FALSE;
}

/* Allow delete of historical data */
UA_Boolean Open62541::Server::allowHistoryUpdateDeleteRawModifiedHandler(UA_Server* server,
                                                                         UA_AccessControl* ac,
                                                                         const UA_NodeId* sessionId,
                                                                         void* sessionContext,
                                                                         const UA_NodeId* nodeId,
                                                                         UA_DateTime startTimestamp,
                                                                         UA_DateTime endTimestamp,
                                                                         bool isDeleteModified)
{
    Server* p = Open62541::Server::findServer(server);
    if (p) {
        return p->allowHistoryUpdateDeleteRawModified(ac,
                                                      sessionId,
                                                      sessionContext,
                                                      nodeId,
                                                      startTimestamp,
                                                      endTimestamp,
                                                      isDeleteModified);
    }
    return UA_FALSE;
}

/* Allow browsing a node */
UA_Boolean Open62541::Server::allowBrowseNodeHandler(UA_Server* server,
                                                     UA_AccessControl* ac,
                                                     const UA_NodeId* sessionId,
                                                     void* sessionContext,
                                                     const UA_NodeId* nodeId,
                                                     void* nodeContext)
{
    Server* p = Open62541::Server::findServer(server);
    if (p) {
        return (p->allowBrowseNode(ac, sessionId, sessionContext, nodeId, nodeContext)) ? UA_TRUE : UA_FALSE;
    }
    return UA_FALSE;
}

#ifdef UA_ENABLE_SUBSCRIPTIONS
/* Allow transfer of a subscription to another session. The Server shall
 * validate that the Client of that Session is operating on behalf of the
 * same user */
UA_Boolean Open62541::Server::allowTransferSubscriptionHandler(UA_Server* server,
                                                               UA_AccessControl* ac,
                                                               const UA_NodeId* oldSessionId,
                                                               void* oldSessionContext,
                                                               const UA_NodeId* newSessionId,
                                                               void* newSessionContext)
{
    Server* p = Open62541::Server::findServer(server);
    if (p) {
        return (p->allowTransferSubscription(ac, oldSessionId, oldSessionContext, newSessionId, newSessionContext))
                   ? UA_TRUE
                   : UA_FALSE;
    }
    return UA_FALSE;
}

#endif

/*!
    \brief deleteTree
    \param nodeId
    \return
*/
bool Open62541::Server::deleteTree(const NodeId& nodeId)
{
    if (!_server)
        return false;
    NodeIdMap m;  // set of nodes to delete
    browseTree(nodeId, m);
    for (auto i = m.begin(); i != m.end(); i++) {
        {
            UA_NodeId& ni = i->second;
            if (ni.namespaceIndex > 0) {  // namespaces 0  appears to be reserved
                WriteLock l(_mutex);
                UA_Server_deleteNode(_server, i->second, true);
            }
        }
    }
    return lastOK();
}

/*!
    \brief browseTreeCallBack
    \param childId
    \param isInverse
    \param referenceTypeId
    \param handle
    \return
*/

static UA_StatusCode browseTreeCallBack(UA_NodeId childId,
                                        UA_Boolean isInverse,
                                        UA_NodeId /*referenceTypeId*/,
                                        void* handle)
{
    if (!isInverse) {  // not a parent node - only browse forward
        Open62541::UANodeIdList* pl = (Open62541::UANodeIdList*)handle;
        pl->put(childId);
    }
    return UA_STATUSCODE_GOOD;
}

/*!
    \brief Open62541::Client::browseChildren
    \param nodeId
    \param m
    \return
*/

bool Open62541::Server::browseChildren(const UA_NodeId& nodeId, NodeIdMap& m)
{
    if (!_server)
        return false;
    Open62541::UANodeIdList l;
    {

        WriteLock ll(_mutex);
        UA_Server_forEachChildNodeCall(_server, nodeId, browseTreeCallBack, &l);  // get the childlist
    }
    for (int i = 0; i < int(l.size()); i++) {
        if (l[i].namespaceIndex == nodeId.namespaceIndex) {  // only in same namespace
            std::string s = Open62541::toString(l[i]);
            if (m.find(s) == m.end()) {
                m.put(l[i]);
                browseChildren(l[i], m);  // recurse no duplicates
            }
        }
    }
    return lastOK();
}

/*!
    \brief Open62541::Client::browseTree
    \param nodeId
    \param tree
    \return
*/
bool Open62541::Server::browseTree(const Open62541::NodeId& nodeId, Open62541::UANodeTree& tree)
{
    // form a heirachical tree of nodes given node is not added to tree
    return browseTree(nodeId.get(), tree.rootNode());
}

/*!
    \brief Open62541::Client::browseTree
    \param nodeId
    \param node
    \return
*/
bool Open62541::Server::browseTree(const UA_NodeId& nodeId, Open62541::UANode* node)
{
    if (!_server)
        return false;
    // form a heirachical tree of nodes
    Open62541::UANodeIdList l;  // shallow copy node IDs and take ownership
    {
        WriteLock ll(_mutex);
        UA_Server_forEachChildNodeCall(_server, nodeId, browseTreeCallBack, &l);  // get the childlist
    }
    for (int i = 0; i < int(l.size()); i++) {
        if (l[i].namespaceIndex > 0) {
            QualifiedName outBrowseName;
            {
                WriteLock ll(_mutex);
                _lastError = __UA_Server_read(_server, &l[i], UA_ATTRIBUTEID_BROWSENAME, outBrowseName);
            }
            if (_lastError == UA_STATUSCODE_GOOD) {
                std::string s = toString(outBrowseName.get().name);  // get the browse name and leak key
                NodeId nId    = l[i];                                // deep copy
                UANode* n     = node->createChild(s);                // create the node
                n->setData(nId);
                browseTree(l[i], n);
            }
        }
    }
    return lastOK();
}

/*!
    \brief Open62541::Client::browseTree
    \param nodeId
    \param tree
    \return
*/
bool Open62541::Server::browseTree(const NodeId& nodeId, NodeIdMap& m)
{
    m.put(nodeId);
    return browseChildren(nodeId, m);
}

/*!
    \brief Open62541::Server::terminate
*/
void Open62541::Server::terminate()
{
    if (_server) {
        //
        _timerMap.clear();
#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
        _conditionMap.clear();
#endif
        UA_Server_run_shutdown(_server);
        UA_Server_delete(_server);
        _serverMap.erase(_server);
        _server = nullptr;
    }
}

/*!
    \brief Open62541::Server::start
    \param iterate
*/
void Open62541::Server::start()
{  // start the server
    if (!_running) {
        _running = true;
        if (_server) {
            _serverMap[_server] = this;  // map for call backs
            UA_Server_run_startup(_server);
            initialise();
            while (_running) {
                {
                    UA_Server_run_iterate(_server, true);
                }
                process();  // called from time to time - Only safe places to access server are in process() and
                            // callbacks
            }
            terminate();
        }
        _running = false;
    }
}

/*!
    \brief Open62541::Server::stop
*/
void Open62541::Server::stop()
{  // stop the server
    _running = false;
}

/*!
    \brief Open62541::Server::initialise
*/
void Open62541::Server::initialise()
{
    // called after the server object has been created but before run has been called
    // load configuration files and set up the address space
    // create namespaces and endpoints
    // set up methods and stuff
}

/*!
    \brief NodeIdFromPath
    \param path
    \param nameSpaceIndex
    \param nodeId
    \return
*/
bool Open62541::Server::nodeIdFromPath(const NodeId& start, const Path& path, NodeId& nodeId)
{
    //
    nodeId = start;
    //
    int level = 0;
    if (path.size() > 0) {
        ServerBrowser b(*this);
        while (level < int(path.size())) {
            b.browse(nodeId);
            auto i = b.find(path[level]);
            if (i == b.list().end())
                return false;
            level++;
            nodeId = (*i).childId;
        }
    }
    return level == int(path.size());
}

/*!
    \brief createPath
    \param start
    \param path
    \param nameSpaceIndex
    \param nodeId
    \return
*/
bool Open62541::Server::createFolderPath(const NodeId& start, const Path& path, int nameSpaceIndex, NodeId& nodeId)
{
    // nodeId is a shallow copy - do not delete and is volatile
    // create folder path first then add varaibles to path's end leaf
    // create folder path first then add varaibles to path's end leaf
    //
    UA_NodeId n = start.get();  // use node ids to browse with
    int level   = 0;
    if (path.size() > 0) {
        ServerBrowser b(*this);
        while (level < int(path.size())) {
            b.browse(n);
            auto i = b.find(path[level]);
            if (i == b.list().end())
                break;
            level++;
            n = (*i).childId;  // shallow copy
        }
        nodeId = n;
        NodeId newNode;
        while (level < int(path.size())) {
            if (!addFolder(nodeId, path[level], NodeId::Null, newNode.notNull(), nameSpaceIndex))
                break;
            nodeId = newNode;  // assign
            level++;
        }
    }
    return level == int(path.size());
}

/*!
    \brief getChild
    \param nameSpaceIndex
    \param childName
    \return
*/
bool Open62541::Server::getChild(const NodeId& start, const std::string& childName, NodeId& ret)
{
    Path p;
    p.push_back(childName);
    return nodeIdFromPath(start, p, ret);
}

/*!
    \brief Open62541::Server::addFolder
    \param parent
    \param nameSpaceIndex
    \param childName
    \return
*/
bool Open62541::Server::addFolder(const NodeId& parent,
                                  const std::string& childName,
                                  const NodeId& nodeId,
                                  NodeId& newNode,
                                  int nameSpaceIndex)
{
    if (!_server)
        return false;
    if (nameSpaceIndex == 0)
        nameSpaceIndex = parent.nameSpaceIndex();  // inherit parent by default
    QualifiedName qn(nameSpaceIndex, childName);
    ObjectAttributes attr;
    attr.setDefault();
    attr.setDisplayName(childName);
    attr.setDescription(childName);
    WriteLock l(_mutex);
    _lastError = UA_Server_addObjectNode(_server,
                                         nodeId,
                                         parent,
                                         NodeId::Organizes,
                                         qn,
                                         NodeId::FolderType,
                                         attr.get(),
                                         NULL,
                                         newNode.isNull() ? nullptr : newNode.clearRef());
    return lastOK();
}

/*!
    \brief Open62541::Server::addFolder::addVariable
    \param parent
    \param nameSpaceIndex
    \param childName
    \return
*/
bool Open62541::Server::addVariable(const NodeId& parent,
                                    const std::string& childName,
                                    const Variant& value,
                                    const NodeId& nodeId,
                                    NodeId& newNode,
                                    NodeContext* c,
                                    int nameSpaceIndex)
{
    if (!_server)
        return false;
    if (nameSpaceIndex == 0)
        nameSpaceIndex = parent.nameSpaceIndex();  // inherit parent by default

    VariableAttributes var_attr;
    var_attr.setDefault();
    QualifiedName qn(nameSpaceIndex, childName);
    var_attr.setDisplayName(childName);
    var_attr.setDescription(childName);
    var_attr.get().accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    var_attr.setValue(value);
    var_attr.get().dataType = value.get().type->typeId;
    WriteLock l(_mutex);
    _lastError = UA_Server_addVariableNode(_server,
                                           nodeId,
                                           parent,
                                           NodeId::Organizes,
                                           qn,
                                           UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),  // no variable type
                                           var_attr,
                                           c,
                                           newNode.isNull() ? nullptr : newNode.clearRef());
    return lastOK();
}

/*!
    \brief Open62541::Server::addHistoricalVariable
    \param parent
    \param nameSpaceIndex
    \param childName
    \return true on success
*/
bool Open62541::Server::addHistoricalVariable(const NodeId& parent,
                                              const std::string& childName,
                                              const Variant& value,
                                              const NodeId& nodeId,
                                              NodeId& newNode,
                                              NodeContext* c,
                                              int nameSpaceIndex)
{
    if (!_server)
        return false;
    if (nameSpaceIndex == 0)
        nameSpaceIndex = parent.nameSpaceIndex();  // inherit parent by default

    VariableAttributes var_attr;
    var_attr.setDefault();
    QualifiedName qn(nameSpaceIndex, childName);
    var_attr.setDisplayName(childName);
    var_attr.setDescription(childName);
    var_attr.get().accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE | UA_ACCESSLEVELMASK_HISTORYREAD;
    var_attr.setValue(value);
    var_attr.get().dataType    = value.get().type->typeId;
    var_attr.get().historizing = true;
    WriteLock l(_mutex);
    _lastError = UA_Server_addVariableNode(_server,
                                           nodeId,
                                           parent,
                                           NodeId::Organizes,
                                           qn,
                                           UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
                                           var_attr,
                                           c,
                                           newNode.isNull() ? nullptr : newNode.clearRef());
    return lastOK();
}

/*!
    \brief Open62541::Server::addProperty
    \param parent
    \param key
    \param value
    \param nodeId
    \param newNode
    \param c
    \param nameSpaceIndex
    \return
*/
bool Open62541::Server::addProperty(const NodeId& parent,
                                    const std::string& key,
                                    const Variant& value,
                                    const NodeId& nodeId,
                                    NodeId& newNode,
                                    NodeContext* c,
                                    int nameSpaceIndex)
{
    if (!_server)
        return false;
    VariableAttributes var_attr;
    var_attr.setDefault();
    QualifiedName qn(nameSpaceIndex, key);
    var_attr.setDisplayName(key);
    var_attr.setDescription(key);
    var_attr.get().accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    var_attr.setValue(value);
    _lastError = UA_Server_addVariableNode(_server,
                                           nodeId,
                                           parent,
                                           UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY),
                                           qn,
                                           UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
                                           var_attr,
                                           c,
                                           newNode.isNull() ? nullptr : newNode.clearRef());
    return lastOK();
}

/*!
    \brief Open62541::Server::serverOnNetworkCallback
    \param serverNetwork
    \param isServerAnnounce
    \param isTxtReceived
    \param data
*/
void Open62541::Server::serverOnNetworkCallback(const UA_ServerOnNetwork* serverNetwork,
                                                UA_Boolean isServerAnnounce,
                                                UA_Boolean isTxtReceived,
                                                void* data)
{
    Server* p = (Server*)(data);
    if (p)
        p->serverOnNetwork(serverNetwork, isServerAnnounce, isTxtReceived);
}

/*!
    \brief Open62541::Server::registerServerCallback
    \param registeredServer
    \param data
*/
void Open62541::Server::registerServerCallback(const UA_RegisteredServer* registeredServer, void* data)
{
    Server* p = (Server*)(data);
    if (p)
        p->registerServer(registeredServer);
}

bool Open62541::Server::registerDiscovery(Client& client, const std::string& semaphoreFilePath)
{
    _lastError = UA_Server_register_discovery(_server,
                                              client.client(),
                                              semaphoreFilePath.empty() ? nullptr : semaphoreFilePath.c_str());
    return lastOK();
}

/*!
    \brief unregisterDiscovery
    \return  true on success
*/
bool Open62541::Server::unregisterDiscovery(Client& client)
{
    if (!server())
        return false;
    _lastError = UA_Server_unregister_discovery(server(), client.client());
    return lastOK();
}

/*!
    \brief addPeriodicServerRegister
    \param discoveryServerUrl
    \param intervalMs
    \param delayFirstRegisterMs
    \param periodicCallbackId
    \return true on success
*/
bool Open62541::Server::addPeriodicServerRegister(
    const std::string& discoveryServerUrl,  // url must persist - that is be static
    Client& client,
    UA_UInt64& periodicCallbackId,
    UA_UInt32 intervalMs,  // default to 10 minutes
    UA_UInt32 delayFirstRegisterMs)
{
    if (!server())
        return false;
    _lastError = UA_Server_addPeriodicServerRegisterCallback(server(),
                                                             client.client(),
                                                             discoveryServerUrl.c_str(),
                                                             intervalMs,
                                                             delayFirstRegisterMs,
                                                             &periodicCallbackId);
    //
    if (lastOK()) {
        _discoveryList[periodicCallbackId] = discoveryServerUrl;
    }
    //
    return lastOK();
}
