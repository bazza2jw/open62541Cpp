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
#include <open62541cpp/nodecontext.h>
#include <open62541cpp/open62541server.h>

// set of contexts
Open62541::RegisteredNodeContext::NodeContextMap Open62541::RegisteredNodeContext::_map;

// prepared objects
UA_DataSource Open62541::NodeContext::_dataSource = {Open62541::NodeContext::readDataSource,
                                                     Open62541::NodeContext::writeDataSource};

UA_ValueCallback Open62541::NodeContext::_valueCallback = {Open62541::NodeContext::readValueCallback,
                                                           Open62541::NodeContext::writeValueCallback};

UA_NodeTypeLifecycle Open62541::NodeContext::_nodeTypeLifeCycle = {Open62541::NodeContext::typeConstructor,
                                                                   Open62541::NodeContext::typeDestructor};

//
// Default Datavalue
//
static Open62541::Variant defaultValue("Undefined");

/*!
 * \brief Open62541::NodeContext::typeConstructor
 * \param server
 * \param sessionId
 * \param sessionContext
 * \param typeNodeId
 * \param typeNodeContext
 * \param nodeId
 * \param nodeContext
 * \return error code
 */
UA_StatusCode Open62541::NodeContext::typeConstructor(UA_Server* server,
                                                      const UA_NodeId* /*sessionId*/,
                                                      void* /*sessionContext*/,
                                                      const UA_NodeId* typeNodeId,
                                                      void* /*typeNodeContext*/,
                                                      const UA_NodeId* nodeId,
                                                      void** nodeContext)
{
    UA_StatusCode ret = (UA_StatusCode)(-1);
    if (server && nodeId && typeNodeId) {
        NodeContext* p = (NodeContext*)(*nodeContext);
        if (p) {
            //
            Server* s = Server::findServer(server);
            if (s) {
                NodeId n;
                n = *nodeId;
                NodeId t;
                t = *typeNodeId;
                //
                if (p->typeConstruct(*s, n, t))
                    ret = UA_STATUSCODE_GOOD;
            }
        }
    }
    return ret;
}

/* Can be NULL. May replace the nodeContext. */
/*!
 * \brief Open62541::NodeContext::typeDestructor
 * \param server
 * \param sessionId
 * \param sessionContext
 * \param typeNodeId
 * \param typeNodeContext
 * \param nodeId
 * \param nodeContext
 */
void Open62541::NodeContext::typeDestructor(UA_Server* server,
                                            const UA_NodeId* /*sessionId*/,
                                            void* /*sessionContext*/,
                                            const UA_NodeId* typeNodeId,
                                            void* /*typeNodeContext*/,
                                            const UA_NodeId* nodeId,
                                            void** nodeContext)
{
    if (server && nodeId && typeNodeId) {
        NodeContext* p = (NodeContext*)(*nodeContext);
        if (p) {
            //
            Server* s = Server::findServer(server);
            if (s) {
                NodeId n;
                n = *nodeId;
                NodeId t;
                t = *typeNodeId;
                //
                p->typeDestruct(*s, n, t);
            }
        }
    }
}

/*!
 * \brief Open62541::NodeContext::setTypeLifeCycle
 * \param server
 * \param n
 * \return
 */
bool Open62541::NodeContext::setTypeLifeCycle(Server& server, NodeId& n)
{
    _lastError = UA_Server_setNodeTypeLifecycle(server.server(), n, _nodeTypeLifeCycle);
    return lastOK();
}

/*!
 * \brief Open62541::NodeContext::setAsDataSource
 * \param server
 * \param n
 * \return
 */
bool Open62541::NodeContext::setAsDataSource(Server& server, NodeId& n)
{
    // Make this context handle the data source calls
    _lastError = UA_Server_setVariableNode_dataSource(server.server(), n, _dataSource);
    return lastOK();
}

/*!
 * \brief readDataSource
 * \param server
 * \param sessionId
 * \param sessionContext
 * \param nodeId
 * \param nodeContext
 * \param includeSourceTimeStamp
 * \param range
 * \param value
 * \return
 */
UA_StatusCode Open62541::NodeContext::readDataSource(UA_Server* server,
                                                     const UA_NodeId* /*sessionId*/,
                                                     void* /*sessionContext*/,
                                                     const UA_NodeId* nodeId,
                                                     void* nodeContext,
                                                     UA_Boolean includeSourceTimeStamp,
                                                     const UA_NumericRange* range,
                                                     UA_DataValue* value)
{
    UA_StatusCode ret = UA_STATUSCODE_GOOD;
    if (nodeContext) {
        NodeContext* p = (NodeContext*)(nodeContext);  // require node contexts to be NULL or NodeContext objects
        Server* s      = Server::findServer(server);
        if (s && p && nodeId && value) {
            NodeId n;
            n = *nodeId;
            if (!p->readData(*s, n, range, *value)) {
                ret = UA_STATUSCODE_BADDATAUNAVAILABLE;
            }
            else {
                if (includeSourceTimeStamp) {
                    value->hasServerTimestamp = true;
                    value->sourceTimestamp    = UA_DateTime_now();
                }
            }
        }
    }
    return ret;
}

/*!
 * \brief writeDataSource
 * \param server
 * \param sessionId
 * \param sessionContext
 * \param nodeId
 * \param nodeContext
 * \param range
 * \param value
 * \return
 */
UA_StatusCode Open62541::NodeContext::writeDataSource(UA_Server* server,
                                                      const UA_NodeId* /*sessionId*/,
                                                      void* /*sessionContext*/,
                                                      const UA_NodeId* nodeId,
                                                      void* nodeContext,
                                                      const UA_NumericRange* range,  // can be null
                                                      const UA_DataValue* value)
{
    UA_StatusCode ret = UA_STATUSCODE_GOOD;
    if (nodeContext) {
        NodeContext* p = (NodeContext*)(nodeContext);  // require node contexts to be NULL or NodeContext objects
        Server* s      = Server::findServer(server);
        if (s && p && nodeId && value) {
            NodeId n;
            n = *nodeId;
            if (!p->writeData(*s, n, range, *value)) {
                ret = UA_STATUSCODE_BADDATAUNAVAILABLE;
            }
        }
    }
    return ret;
}

/*!
 * \brief Open62541::NodeContext::setValueCallback
 * \param server
 * \param n
 * \return
 */
bool Open62541::NodeContext::setValueCallback(Open62541::Server& server, NodeId& n)
{
    _lastError = UA_Server_setVariableNode_valueCallback(server.server(), n, _valueCallback);
    return lastOK();
}
// Value Callbacks
/*!
 * \brief readValueCallback
 * \param server
 * \param sessionId
 * \param sessionContext
 * \param nodeid
 * \param nodeContext
 * \param range
 * \param value
 */
void Open62541::NodeContext::readValueCallback(UA_Server* server,
                                               const UA_NodeId* /*sessionId*/,
                                               void* /*sessionContext*/,
                                               const UA_NodeId* nodeId,
                                               void* nodeContext,
                                               const UA_NumericRange* range,  // can be null
                                               const UA_DataValue* value)
{
    if (nodeContext) {
        NodeContext* p = (NodeContext*)(nodeContext);  // require node contexts to be NULL or NodeContext objects
        Server* s      = Server::findServer(server);
        if (s && p && nodeId && value) {
            NodeId n = *nodeId;
            p->readValue(*s, n, range, value);
        }
    }
}

/*!
 * \brief writeValueCallback
 * \param server
 * \param sessionId
 * \param sessionContext
 * \param nodeId
 * \param nodeContext
 * \param range
 * \param data
 */
void Open62541::NodeContext::writeValueCallback(UA_Server* server,
                                                const UA_NodeId* /*sessionId*/,
                                                void* /*sessionContext*/,
                                                const UA_NodeId* nodeId,
                                                void* nodeContext,
                                                const UA_NumericRange* range,  // can be null
                                                const UA_DataValue* value)
{
    if (nodeContext) {
        NodeContext* p = (NodeContext*)(nodeContext);  // require node contexts to be NULL or NodeContext objects
        Server* s      = Server::findServer(server);
        if (s && p && nodeId && value) {
            NodeId n = *nodeId;
            p->writeValue(*s, n, range, *value);
        }
    }
}
