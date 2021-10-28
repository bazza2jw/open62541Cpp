/*
 * Copyright (C) 2017 -  B. J. Hill
 *
 * This file is part of open62541 C++ classes. open62541 C++ classes are free software: you can
 * redistribute it and/or modify it under the terms of the Mozilla Public
 * License v2.0 as stated in the LICENSE file provided with open62541.
 *
 * open62541 C++ classes are distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.
 */
#include <open62541cpp/open62541client.h>
#include <open62541cpp/clientbrowser.h>

/*!
 * \brief subscriptionInactivityCallback
 * \param client
 * \param subscriptionId
 * \param subContext
 */
void Open62541::Client::subscriptionInactivityCallback(UA_Client* client, UA_UInt32 subscriptionId, void* subContext)
{
    Client* p = (Client*)(UA_Client_getContext(client));
    if (p != nullptr) {
        p->subscriptionInactivity(subscriptionId, subContext);
    }
}

/*!
 * \brief Open62541::Client::asyncServiceCallback
 * \param client
 * \param userdata
 * \param requestId
 * \param response
 * \param responseType
 */
void Open62541::Client::asyncServiceCallback(UA_Client* client,
                                             void* userdata,
                                             UA_UInt32 requestId,
                                             void* response,
                                             const UA_DataType* responseType)
{
    Client* p = (Client*)(UA_Client_getContext(client));
    if (p != nullptr) {
        p->asyncService(userdata, requestId, response, responseType);
    }
}

/*!
 * \brief Open62541::Client::stateCallback
 * \param client
 * \param clientState
 */
void Open62541::Client::stateCallback(UA_Client* client,
                                      UA_SecureChannelState channelState,
                                      UA_SessionState sessionState,
                                      UA_StatusCode connectStatus)
{
    Client* p = (Client*)(UA_Client_getContext(client));
    if (p != nullptr) {
        p->stateChange(channelState, sessionState, connectStatus);
    }
}

/*!
    \brief Open62541::Client::deleteTree
    \param nodeId
    \return
*/
void Open62541::Client::deleteTree(const NodeId& nodeId)
{
    NodeIdMap m;
    browseTree(nodeId, m);
    for (auto& i : m) {
        UA_NodeId& ni = i.second;
        if (ni.namespaceIndex > 0) {  // namespace 0 appears to be reserved
            WriteLock l(_mutex);
            throw_bad_status(UA_Client_deleteNode(client_or_throw(), i.second, true));
        }
    }
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
void Open62541::Client::browseChildren(const UA_NodeId& nodeId, NodeIdMap& m)
{
    Open62541::UANodeIdList l;
    {
        WriteLock ll(mutex());
        throw_bad_status(
            UA_Client_forEachChildNodeCall(client_or_throw(), nodeId, browseTreeCallBack, &l));  // get the childlist
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
}

/*!
    \brief Open62541::Client::browseTree
    \param nodeId
    \param tree
    \return
*/
void Open62541::Client::browseTree(const Open62541::NodeId& nodeId, Open62541::UANodeTree& tree)
{
    // form a heirachical tree of nodes given node is added to tree
    tree.root().setData(nodeId);  // set the root of the tree
    browseTree(nodeId.get(), tree.rootNode());
}

/*!
    \brief Open62541::Client::browseTree
    \param nodeId
    \param node
    \return
*/
void Open62541::Client::browseTree(const UA_NodeId& nodeId, Open62541::UANode* node)
{
    // form a heirachical tree of nodes
    Open62541::UANodeIdList l;
    {
        WriteLock ll(mutex());
        UA_Client_forEachChildNodeCall(client_or_throw(), nodeId, browseTreeCallBack, &l);  // get the childlist
    }
    for (int i = 0; i < int(l.size()); i++) {
        if (l[i].namespaceIndex > 0) {
            QualifiedName outBrowseName;
            {
                WriteLock ll(mutex());
                throw_bad_status(__UA_Client_readAttribute(client_or_throw(),
                                                           &l[i],
                                                           UA_ATTRIBUTEID_BROWSENAME,
                                                           outBrowseName,
                                                           &UA_TYPES[UA_TYPES_QUALIFIEDNAME]));
            }

            std::string s = toString(outBrowseName.get().name);  // get the browse name and leaf key
            NodeId nId    = l[i];                                // deep copy
            UANode* n     = node->createChild(s);                // create the node
            n->setData(nId);
            browseTree(l[i], n);
        }
    }
}

/*!
    \brief Open62541::Client::browseTree
    \param nodeId
    \param tree
    \return
*/
void Open62541::Client::browseTree(const NodeId& nodeId, NodeIdMap& m)
{
    m.put(nodeId);
    browseChildren(nodeId, m);
}

/*!
    \brief Open62541::Client::getEndpoints
    \param serverUrl
    \param list
    \return
*/
void Open62541::Client::getEndpoints(const std::string& serverUrl, std::vector<std::string>& list)
{
    UA_EndpointDescription* endpointDescriptions = nullptr;
    size_t endpointDescriptionsSize              = 0;

    {
        WriteLock l(_mutex);
        throw_bad_status(UA_Client_getEndpoints(client_or_throw(),
                                                serverUrl.c_str(),
                                                &endpointDescriptionsSize,
                                                &endpointDescriptions));
    }
    for (int i = 0; i < int(endpointDescriptionsSize); i++) {
        list.push_back(toString(endpointDescriptions[i].endpointUrl));
    }
}

/*!
    \brief NodeIdFromPath
    \param path
    \param nameSpaceIndex
    \param nodeId
    \return
*/
void Open62541::Client::nodeIdFromPath(const NodeId& start, const Path& path, NodeId& nodeId)
{
    if (path.empty()) {
        throw StringException("Node not found due to empty path.");
    }
    // nodeId is a shallow copy - do not delete and is volatile
    UA_NodeId n = start.get();

    int level = 0;

    Open62541::ClientBrowser b(*this);
    while (level < int(path.size())) {
        b.browse(n);
        auto i = b.find(path[level]);
        if (i == b.list().end()) {
            throw StringException("Node not found");
        }
        level++;
        n = (*i).childId;
    }
    nodeId = n;  // deep copy
}

/*!
    \brief createPath
    \param start
    \param path
    \param nameSpaceIndex
    \param nodeId
    \return
*/
void Open62541::Client::createFolderPath(const NodeId& start, const Path& path, int nameSpaceIndex, NodeId& nodeId)
{
    if (path.empty()) {
        return;
    }
    //
    // create folder path first then add varaibles to path's end leaf
    //
    UA_NodeId n = start.get();
    //
    int level = 0;
    Open62541::ClientBrowser b(*this);
    while (level < int(path.size())) {
        b.browse(n);
        auto i = b.find(path[level]);
        if (i == b.list().end()) {
            break;
        }
        level++;
        n = (*i).childId;  // shallow copy
    }
    if (level == int(path.size())) {
        nodeId = n;
    }
    else {
        NodeId nf(nameSpaceIndex, 0);  // auto generate NODE id
        nodeId = n;
        NodeId newNode;
        while (level < int(path.size())) {
            addFolder(nodeId, path[level], nf, newNode.notNull(), nameSpaceIndex);
            nodeId = newNode;  // assign
            level++;
        }
    }
}

/*!
    \brief getChild
    \param nameSpaceIndex
    \param childName
    \return
*/
void Open62541::Client::getChild(const NodeId& start, const std::string& childName, NodeId& ret)
{
    Path p;
    p.push_back(childName);
    nodeIdFromPath(start, p, ret);
}

/*!
    \brief Open62541::Client::addFolder
    \param parent
    \param nameSpaceIndex
    \param childName
    \return
*/
void Open62541::Client::addFolder(const NodeId& parent,
                                  const std::string& childName,
                                  const NodeId& nodeId,
                                  NodeId& newNode,
                                  int nameSpaceIndex)
{
    WriteLock l(_mutex);
    //
    if (nameSpaceIndex == 0) {
        nameSpaceIndex = parent.nameSpaceIndex();  // inherit parent by default
    }
    //
    QualifiedName qn(nameSpaceIndex, childName);
    ObjectAttributes attr;
    attr.setDisplayName(childName);
    attr.setDescription(childName);
    //
    throw_bad_status(UA_Client_addObjectNode(client_or_throw(),
                                             nodeId,
                                             parent,
                                             NodeId::Organizes,
                                             qn,
                                             NodeId::FolderType,
                                             attr.get(),
                                             newNode.isNull() ? nullptr : newNode.ref()));
}

/*!
    \brief Open62541::Client::addFolder::addVariable
    \param parent
    \param nameSpaceIndex
    \param childName
    \return
*/
void Open62541::Client::addVariable(const NodeId& parent,
                                    const std::string& childName,
                                    const Variant& value,
                                    const NodeId& nodeId,
                                    NodeId& newNode,
                                    int nameSpaceIndex)
{
    WriteLock l(_mutex);
    if (nameSpaceIndex == 0) {
        nameSpaceIndex = parent.nameSpaceIndex();  // inherit parent by default
    }
    VariableAttributes var_attr;
    QualifiedName qn(nameSpaceIndex, childName);
    var_attr.setDisplayName(childName);
    var_attr.setDescription(childName);
    var_attr.setValue(value);
    throw_bad_status(UA_Client_addVariableNode(client_or_throw(),
                                               nodeId,  // Assign new/random NodeID
                                               parent,
                                               NodeId::Organizes,
                                               qn,
                                               UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),  // no variable type
                                               var_attr,
                                               newNode.isNull() ? nullptr : newNode.ref()));
}

/*!
 * \brief Open62541::Client::addProperty
 * \param parent
 * \param key
 * \param value
 * \param nodeId
 * \param newNode
 * \return
 */
void Open62541::Client::addProperty(const NodeId& parent,
                                    const std::string& key,
                                    const Variant& value,
                                    const NodeId& nodeId,
                                    NodeId& newNode,
                                    int nameSpaceIndex)
{
    WriteLock l(_mutex);
    if (nameSpaceIndex == 0) {
        nameSpaceIndex = parent.nameSpaceIndex();  // inherit parent by default
    }
    VariableAttributes var_attr;
    QualifiedName qn(nameSpaceIndex, key);
    var_attr.setDisplayName(key);
    var_attr.setDescription(key);
    var_attr.setValue(value);
    throw_bad_status(UA_Client_addVariableNode(client_or_throw(),
                                               nodeId,  // Assign new/random NodeID
                                               parent,
                                               UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY),
                                               qn,
                                               UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),  // no variable type
                                               var_attr,
                                               newNode.isNull() ? nullptr : newNode.ref()));
}

/*!
 * \brief Open62541::Client::stateChange
 * \param channelState
 * \param sessionState
 * \param connectStatus
 */
void Open62541::Client::stateChange(UA_SecureChannelState channelState,
                                    UA_SessionState sessionState,
                                    UA_StatusCode connectStatus)
{

    _channelState  = channelState;
    _sessionState  = sessionState;
    _connectStatus = connectStatus;

    if (connectStatus != 0u) {
        connectFail();
    }

    if (_lastSessionState != sessionState) {
        switch (sessionState) {
            case UA_SESSIONSTATE_CLOSED:
                SessionStateClosed();

                break;
            case UA_SESSIONSTATE_CREATE_REQUESTED:
                SessionStateCreateRequested();

                break;
            case UA_SESSIONSTATE_CREATED:
                SessionStateCreated();

                break;
            case UA_SESSIONSTATE_ACTIVATE_REQUESTED:
                SessionStateActivateRequested();
                break;
            case UA_SESSIONSTATE_ACTIVATED:
                SessionStateActivated();
                break;
            case UA_SESSIONSTATE_CLOSING:
                SessionStateClosing();
                break;
            default:
                break;
        }
        _lastSessionState = sessionState;
    }

    if (_lastSecureChannelState != channelState) {

        switch (channelState) {
            case UA_SECURECHANNELSTATE_CLOSED:
                SecureChannelStateClosed();
                break;
            case UA_SECURECHANNELSTATE_HEL_SENT:
                SecureChannelStateHelSent();
                break;
            case UA_SECURECHANNELSTATE_HEL_RECEIVED:
                SecureChannelStateHelReceived();
                break;
            case UA_SECURECHANNELSTATE_ACK_SENT:
                SecureChannelStateAckSent();
                break;
            case UA_SECURECHANNELSTATE_ACK_RECEIVED:
                SecureChannelStateAckReceived();
                break;
            case UA_SECURECHANNELSTATE_OPN_SENT:
                SecureChannelStateOpenSent();
                break;
            case UA_SECURECHANNELSTATE_OPEN:
                SecureChannelStateOpen();
                break;
            case UA_SECURECHANNELSTATE_CLOSING:
                SecureChannelStateClosing();
                break;
            default:
                break;
        }
        _lastSecureChannelState = channelState;
    }
}
