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
#ifndef SERVERNODETREE_H
#define SERVERNODETREE_H
#include <open62541cpp/open62541objects.h>
#include <open62541cpp/open62541server.h>
namespace Open62541 {

/*!
    \brief The ServerNodeTree class
*/
class UA_EXPORT ServerNodeTree : public UANodeTree
{
    Server& _server;     // server
    int _nameSpace = 2;  // sname space index we create nodes in
public:
    /*!
        \brief setNameSpace
        \param i
        \return
    */
    void setNameSpace(int i) { _nameSpace = i; }
    /*!
        \brief nameSpace
        \return
    */
    int nameSpace() const { return _nameSpace; }
    /*!
        \brief ServerNodeTree
        \param s
        \param parent
        \param ns
    */
    ServerNodeTree(Server& s, NodeId& parent, int ns = 2);
    // client and server have different methods - TO DO unify client and server - and template
    // only deal with value nodes and folders - for now

    /*!
     * \brief ~ServerNodeTree
     */
    virtual ~ServerNodeTree();
    /*!
        \brief addFolderNode
        \param parent
        \param s
        \return
    */
    virtual bool addFolderNode(NodeId& parent, const std::string& s, NodeId& no);
    /*!
        \brief addValueNode
        \return
    */
    virtual bool addValueNode(NodeId& parent, const std::string& s, NodeId& no, Variant& v);
    /*!
        \brief getValue
        \return
    */
    virtual bool getValue(NodeId& n, Variant& v);
    /*!
        \brief setValue
        \return
    */
    virtual bool setValue(NodeId& n, Variant& v);
};

}  // namespace Open62541
#endif  // SERVERNODETREE_H
