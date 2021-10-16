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
#include <open62541cpp/servermethod.h>
#include <open62541cpp/open62541server.h>
/*!
    \brief Open62541::ServerMethod::methodCallback
    \param handle
    \param objectId
    \param inputSize
    \param input
    \param outputSize
    \param output
    \return status code
*/
UA_StatusCode Open62541::ServerMethod::methodCallback(UA_Server* server,
                                                      const UA_NodeId* /*sessionId*/,
                                                      void* /*sessionContext*/,
                                                      const UA_NodeId* /*methodId*/,
                                                      void* methodContext,  // references the handler
                                                      const UA_NodeId* objectId,
                                                      void* /*objectContext*/,
                                                      size_t inputSize,
                                                      const UA_Variant* input,
                                                      size_t outputSize,
                                                      UA_Variant* output)
{
    UA_StatusCode ret = UA_STATUSCODE_GOOD;
    if (methodContext) {
        Server* s = Server::findServer(server);
        if (s) {
            Open62541::ServerMethod* p = (Open62541::ServerMethod*)methodContext;
            if (p->_func) {
                return p->_func(*s, objectId, inputSize, input, outputSize, output);  // was the functor defined
            }
            ret = p->callback(*s,
                              objectId,
                              inputSize,
                              input,
                              outputSize,
                              output);  // adding a method allocates in/out variable space
        }
    }
    return ret;
}

/*!
    \brief Open62541::ServerMethod::ServerMethod
    \param s
    \param n
    \param nInputs
    \param nOutputs
*/
Open62541::ServerMethod::ServerMethod(const std::string& n, int nInputs, int nOutputs)
    : NodeContext(n)
{
    _in.resize(nInputs + 1);  // create parameter space
    _out.resize(nOutputs + 1);
}

Open62541::ServerMethod::ServerMethod(const std::string& n, MethodFunc f, int nInputs, int nOutputs)
    : NodeContext(n)
    , _func(f)
{
    _in.resize(nInputs + 1);  // create parameter space
    _out.resize(nOutputs + 1);
}

/*!
 * \brief setMethodNodeCallBack
 * \param s
 * \param node
 * \return
 */
bool Open62541::ServerMethod::setMethodNodeCallBack(Open62541::Server& s, Open62541::NodeId& node)
{
    return s.server() ? (UA_Server_setMethodNode_callback(s.server(), node, methodCallback) == UA_STATUSCODE_GOOD)
                      : false;
}

/*!
 * \brief addServerMethod
 * \param browseName
 * \param parent
 * \param nodeId
 * \param newNode
 * \param nameSpaceIndex
 * \return
 */
bool Open62541::ServerMethod::addServerMethod(Open62541::Server& s,
                                              const std::string& browseName,
                                              Open62541::NodeId& parent,
                                              Open62541::NodeId& nodeId,
                                              Open62541::NodeId& newNode,
                                              int nameSpaceIndex)
{
    return s.addServerMethod(this, browseName, parent, nodeId, newNode, nameSpaceIndex);
}
