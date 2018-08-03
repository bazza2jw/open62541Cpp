/*
    Copyright (C) 2017 -  B. J. Hill

    This file is part of open62541 C++ classes. open62541 C++ classes are free software: you can
    redistribute it and/or modify it under the terms of the Mozilla Public
    License v2.0 as stated in the LICENSE file provided with open62541.

    open62541 C++ classes are distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
    A PARTICULAR PURPOSE.
*/
#include "open62541server.h"
#include "nodecontext.h"
#include "serverbrowser.h"

// map UA_SERVER to Server objects
Open62541::Server::ServerMap  Open62541::Server::_serverMap;

/*!
    \brief Open62541::Server::findContext
    \param s
    \return
*/
Open62541::NodeContext *Open62541::Server::findContext(const std::string &s) {
    return RegisteredNodeContext::findRef(s); // not all node contexts are registered
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
UA_StatusCode Open62541::Server::constructor(UA_Server *server,
                                             const UA_NodeId * /*sessionId*/, void * /*sessionContext*/,
                                             const UA_NodeId *nodeId, void **nodeContext) {
    UA_StatusCode ret = UA_STATUSCODE_GOOD;
    if (server && nodeId && nodeContext) {
        void *p = *nodeContext;
        NodeContext *cp = (NodeContext *)(p);
        if (cp) {
            Server *s = Server::findServer(server);
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
void Open62541::Server::destructor(UA_Server *server,
                                   const UA_NodeId * /*sessionId*/, void * /*sessionContext*/,
                                   const UA_NodeId *nodeId, void *nodeContext) {
    if (server && nodeId && nodeContext) {
        NodeContext *cp = (NodeContext *)(nodeContext);
        Server *s = Server::findServer(server);
        if (s) {
            NodeId n(*nodeId);
            cp->destruct(*s, n);
        }
    }

}


/*!
    \brief Open62541::Client::deleteTree
    \param nodeId
    \return
*/
bool Open62541::Server::deleteTree(NodeId &nodeId) {
    NodeIdMap m; // set of nodes to delete
    browseTree(nodeId, m);
    for (auto i = m.begin(); i != m.end(); i++) {
        //std::cerr  << "Delete " << i->first << std::endl;
        {
            UA_NodeId &ni =  i->second;
            if (ni.namespaceIndex > 0) { // namespaces 0  appears to be reserved
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

static UA_StatusCode browseTreeCallBack(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId /*referenceTypeId*/, void *handle) {
    if (!isInverse) { // not a parent node - only browse forward
        Open62541::UANodeIdList  *pl = (Open62541::UANodeIdList *)handle;
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

bool Open62541::Server::browseChildren(UA_NodeId &nodeId, NodeIdMap &m) {
    Open62541::UANodeIdList l;
    {

        WriteLock ll(_mutex);
        UA_Server_forEachChildNodeCall(_server, nodeId,  browseTreeCallBack, &l); // get the childlist
    }
    for (int i = 0; i < int(l.size()); i++) {
        if (l[i].namespaceIndex == nodeId.namespaceIndex) { // only in same namespace
            std::string s = Open62541::toString(l[i]);
            if (m.find(s) == m.end()) {
                m.put(l[i]);
                browseChildren(l[i], m); // recurse no duplicates
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
bool Open62541::Server::browseTree(Open62541::NodeId &nodeId, Open62541::UANodeTree &tree) {
    // form a heirachical tree of nodes given node is not added to tree
    return browseTree(nodeId.get(), tree.rootNode());
}

/*!
    \brief Open62541::Client::browseTree
    \param nodeId
    \param node
    \return
*/
bool Open62541::Server::browseTree(UA_NodeId &nodeId, Open62541::UANode *node) {
    // form a heirachical tree of nodes
    Open62541::UANodeIdList l; // shallow copy node IDs and take ownership
    {
        WriteLock ll(_mutex);
        UA_Server_forEachChildNodeCall(_server, nodeId,  browseTreeCallBack, &l); // get the childlist
    }
    for (int i = 0; i < int(l.size()); i++) {
        if (l[i].namespaceIndex > 0) {
            QualifiedName outBrowseName;
            {
                WriteLock ll(_mutex);
                _lastError = __UA_Server_read(_server, &l[i], UA_ATTRIBUTEID_BROWSENAME, outBrowseName);
            }
            if (_lastError == UA_STATUSCODE_GOOD) {
                std::string s = toString(outBrowseName.get().name); // get the browse name and leak key
                NodeId nId = l[i]; // deep copy
                UANode *n = node->createChild(s); // create the node
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
bool Open62541::Server::browseTree(NodeId &nodeId, NodeIdMap &m) {
    m.put(nodeId);
    return browseChildren(nodeId, m);
}

/*!
    \brief Open62541::Server::terminate
*/
void Open62541::Server::terminate() {
    if (_server) {

        // un-register all discovery server links
        for (auto i =  _discoveryList.begin(); i !=  _discoveryList.end(); i++) {
            unregisterDiscovery(i->second);
        }
        //
        UA_Server_run_shutdown(_server);
        UA_Server_delete(_server);
        _serverMap.erase(_server);
        _server = nullptr;
    }
    _discoveryList.clear();
}

/*!
    \brief Open62541::Server::start
    \param iterate
*/
void Open62541::Server::start() { // start the server
    if (!_running) {
        _running = true;
        _server = UA_Server_new(_config);
        if (_server) {
            _serverMap[_server] = this; // map for call backs
            UA_Server_run_startup(_server);
            initialise();
            while (_running) {
                {
                    UA_Server_run_iterate(_server, true);
                }
                process(); // called from time to time - Only safe places to access server are in process() and callbacks
            }
            terminate();
        }
        _running = false;
    }
}

/*!
    \brief Open62541::Server::stop
*/
void Open62541::Server::stop() { // stop the server
    _running = false;
}

/*!
    \brief Open62541::Server::initialise
*/
void Open62541::Server::initialise() {
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
bool Open62541::Server::nodeIdFromPath(NodeId &start, Path &path,  NodeId &nodeId) {
    //
    nodeId = start;
    //
    int level = 0;
    if (path.size() > 0) {
        ServerBrowser b(*this);
        while (level < int(path.size())) {
            b.browse(nodeId);
            auto i = b.find(path[level]);
            if (i == b.list().end()) return false;
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
bool Open62541::Server::createFolderPath(NodeId &start, Path &path, int nameSpaceIndex, NodeId &nodeId) {
    // nodeId is a shallow copy - do not delete and is volatile
    // create folder path first then add varaibles to path's end leaf
    // create folder path first then add varaibles to path's end leaf
    //
    UA_NodeId n = start.get(); // use node ids to browse with
    int level = 0;
    if (path.size() > 0) {
        ServerBrowser b(*this);
        while (level < int(path.size())) {
            b.browse(n);
            auto i = b.find(path[level]);
            if (i == b.list().end())  break;
            level++;
            n = (*i).childId; // shallow copy
        }
        nodeId = n;
        NodeId newNode;
        while (level < int(path.size())) {
            if (!addFolder(nodeId, path[level], NodeId::Null, newNode.notNull(), nameSpaceIndex)) break;
            nodeId = newNode; // assign
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
bool Open62541::Server::getChild(NodeId &start,  const std::string &childName, NodeId &ret) {
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
bool Open62541::Server::addFolder(NodeId &parent, const std::string &childName, NodeId &nodeId,
                                  NodeId &newNode, int nameSpaceIndex) {
    if (nameSpaceIndex == 0) nameSpaceIndex = parent.nameSpaceIndex(); // inherit parent by default
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
                                         newNode.isNull() ? nullptr : newNode.ref());
    return lastOK();
}

/*!
    \brief Open62541::Server::addFolder::addVariable
    \param parent
    \param nameSpaceIndex
    \param childName
    \return
*/
bool Open62541::Server::addVariable(NodeId &parent,  const std::string &childName, Variant &value,
                                    NodeId &nodeId,  NodeId &newNode,  NodeContext *c,  int nameSpaceIndex) {
    if (nameSpaceIndex == 0) nameSpaceIndex = parent.nameSpaceIndex(); // inherit parent by default

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
                                           UA_NODEID_NULL, // no variable type
                                           var_attr,
                                           c,
                                           newNode.isNull() ? nullptr : newNode.ref());
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
bool Open62541::Server::addProperty(NodeId &parent,
                                    const std::string &key,
                                    Variant &value,
                                    NodeId &nodeId,
                                    NodeId &newNode,
                                    NodeContext *c,
                                    int nameSpaceIndex) {

    VariableAttributes var_attr;
    var_attr.setDefault();
    QualifiedName qn(nameSpaceIndex, key);
    var_attr.setDisplayName(key);
    var_attr.setDescription(key);
    var_attr.get().accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    var_attr.setValue(value);
    _lastError = UA_Server_addVariableNode(_server, nodeId,
                                           parent,
                                           UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY),
                                           qn,
                                           UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
                                           var_attr,
                                           c,
                                           newNode.isNull() ? nullptr : newNode.ref());
    return lastOK();
}

/*!
    \brief Open62541::Server::serverOnNetworkCallback
    \param serverNetwork
    \param isServerAnnounce
    \param isTxtReceived
    \param data
*/
void Open62541::Server::serverOnNetworkCallback(const UA_ServerOnNetwork *serverNetwork,
                                                UA_Boolean isServerAnnounce,
                                                UA_Boolean isTxtReceived,
                                                void *data) {
    Server *p = (Server *)(data);
    if (p) p->serverOnNetwork(serverNetwork, isServerAnnounce, isTxtReceived);
}

/*!
    \brief Open62541::Server::registerServerCallback
    \param registeredServer
    \param data
*/
void Open62541::Server::registerServerCallback(const UA_RegisteredServer *registeredServer, void *data) {
    Server *p = (Server *)(data);
    if (p) p->registerServer(registeredServer);
}

