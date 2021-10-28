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
#include <open62541cpp/servernodetree.h>
#include <open62541cpp/open62541server.h>

/*!
    \brief ServerNodeTree
    \param s
    \param parent
    \param ns
*/
Open62541::ServerNodeTree::ServerNodeTree(Server& s, NodeId& parent, int ns)
    : UANodeTree(parent)
    , _server(s)
    , _nameSpace(ns)
{
}

/*!
 * \brief ~ServerNodeTree
 */
Open62541::ServerNodeTree::~ServerNodeTree() = default;

/*!
    \brief addFolderNode
    \param parent
    \param s
    \return
*/
void Open62541::ServerNodeTree::addFolderNode(const NodeId& parent, const std::string& s, NodeId& no)
{
    NodeId ni(_nameSpace, 0);
    _server.addFolder(parent, s, ni, no, _nameSpace);
}
/*!
    \brief addValueNode
    \return
*/
void Open62541::ServerNodeTree::addValueNode(const NodeId& parent, const std::string& s, NodeId& no, const Variant& v)
{
    NodeId ni(_nameSpace, 0);
    _server.addVariable(parent, s, v, ni, no, nullptr, _nameSpace);
}
/*!
    \brief getValue
    \return
*/
void Open62541::ServerNodeTree::getValue(const NodeId& n, Variant& v)
{
    _server.readValue(n, v);
}
/*!
    \brief setValue
    \return
*/
void Open62541::ServerNodeTree::setValue(const NodeId& n, const Variant& v)
{
    _server.writeValue(n, v);
}
