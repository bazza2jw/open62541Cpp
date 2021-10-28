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
#include <open62541cpp/serverobjecttype.h>

#include <utility>

/*!
       \brief Open62541::ServerObjectType::ServerObjectType
       \param n
*/
Open62541::ServerObjectType::ServerObjectType(Server& s, std::string n)
    : _server(s)
    , _name(std::move(n))
{
}
/*!
    \brief ~ServerObjectType
*/
Open62541::ServerObjectType::~ServerObjectType() = default;

/*!
    \brief addBaseObjectType
    \param n
    \param typeId
    \return
*/
void Open62541::ServerObjectType::addBaseObjectType(const std::string& n,
                                                    const NodeId& requestNodeId,
                                                    NodeContext* context)
{
    ObjectTypeAttributes dtAttr;
    QualifiedName qn(_nameSpace, n);
    dtAttr.setDisplayName(n);
    _typeId.notNull();
    _server.addObjectTypeNode(requestNodeId, NodeId::BaseObjectType, NodeId::HasSubType, qn, dtAttr, _typeId, context);
}

/*!
    \brief addDerivedObjectType
    \param n
    \param parent
    \param typeId
    \return
*/
void Open62541::ServerObjectType::addDerivedObjectType(const std::string& n,
                                                       const NodeId& parent,
                                                       NodeId& typeId,
                                                       const NodeId& requestNodeId,
                                                       NodeContext* context)
{
    ObjectTypeAttributes ptAttr;
    ptAttr.setDisplayName(n);
    QualifiedName qn(_nameSpace, n);
    //
    _server.addObjectTypeNode(requestNodeId, parent, NodeId::HasSubType, qn, ptAttr, typeId, context);
}

/*!
    \brief add
    \param server
    \param baseId
    \return
*/
void Open62541::ServerObjectType::addType(const NodeId& nodeId)
{  // base node of type
    addBaseObjectType(_name, nodeId);
    addChildren(_typeId);
}

/*!
    \brief append
    \param server
    \param parent
    \param nodeId
    \return
*/
void Open62541::ServerObjectType::append(const NodeId& parent, NodeId& nodeId, const NodeId& requestNodeId)
{  // derived type - returns node id of append type
    addDerivedObjectType(_name, parent, nodeId, requestNodeId);
    addChildren(nodeId);
}

/*!
    \brief Open62541::ServerObjectType::addInstance
    \param n
    \param parent
    \param nodeId
    \return
*/
void Open62541::ServerObjectType::addInstance(const std::string& n,
                                              const NodeId& parent,
                                              NodeId& nodeId,
                                              const NodeId& requestNodeId,
                                              NodeContext* context)
{
    _server.addInstance(n, requestNodeId, parent, _typeId, nodeId, context);
}
