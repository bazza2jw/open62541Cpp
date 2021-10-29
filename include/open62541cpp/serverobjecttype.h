/*
    Copyright (C) 2017 -  B. J. Hill

    This file is part of open62541 C++ classes. open62541 C++ classes are free software: you can
    redistribute it and/or modify it under the terms of the Mozilla Public
    License v2.0 as stated in the LICENSE file provided with open62541.

    open62541 C++ classes are distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
    A PARTICULAR PURPOSE.
*/
#ifndef SERVEROBJECTTYPE_H
#define SERVEROBJECTTYPE_H
#include <open62541cpp/open62541server.h>
namespace Open62541 {

/*!
    \brief The ServerObjectType class
    Object type handling class - this is factory for object type - operates on a server instance
    The NodeContext is the node life cycle manager

*/
class UA_EXPORT ServerObjectType
{
    Server& _server;
    std::string _name;

    NodeId _typeId;
    int _nameSpace = 2;

public:
    /*!
        \brief ServerObjectType
        \param s
    */
    ServerObjectType(Server& s, std::string n);
    /*!
        \brief ~ServerObjectType
    */
    virtual ~ServerObjectType();
    /*!
        \brief name
        \return
    */
    const std::string& name() { return _name; }

    /*!
        \brief nameSpace
        \return
    */
    int nameSpace() const { return _nameSpace; }
    /*!
        \brief setNameSpace
        \param i
    */
    void setNameSpace(int i) { _nameSpace = i; }
    /*!
        \brief server
        \return
    */
    Server& server() { return _server; }
    /*!
        \brief typeId
        \return
    */
    NodeId& typeId() { return _typeId; }

    /*!
        \brief addBaseObjectType
        \param n
        \param typeId
        \return
    */
    void addBaseObjectType(const std::string& n,
                           const NodeId& requestNodeId = NodeId::Null,
                           NodeContext* context        = nullptr);
    /*!
        \brief addObjectTypeVariable
        \param n
        \param parent
        \param nodeiD
        \param mandatory
        \return
    */
    template <typename T>
    void addObjectTypeVariable(const std::string& n,
                               const NodeId& parent,
                               NodeId& nodeId              = NodeId::Null,
                               NodeContext* context        = nullptr,
                               const NodeId& requestNodeId = NodeId::Null,  // usually want auto generated ids
                               bool mandatory              = true)
    {
        T a{};
        Variant value(a);
        //
        VariableAttributes var_attr;
        var_attr.setDefault();
        var_attr.setDisplayName(n);
        var_attr.setDescription(n);
        var_attr.get().accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
        var_attr.setValue(value);
        var_attr.get().dataType = value.get().type->typeId;
        //
        QualifiedName qn(_nameSpace, n.c_str());
        //
        NodeId newNode;
        newNode.notNull();
        //
        _server.addVariableNode(requestNodeId,
                                parent,
                                NodeId::HasComponent,
                                qn,
                                NodeId::BaseDataVariableType,
                                var_attr,
                                newNode,
                                context);
        if (mandatory) {
            return _server.addReference(newNode,
                                        NodeId::HasModellingRule,
                                        ExpandedNodeId::ModellingRuleMandatory,
                                        true);
        }
        if (!nodeId.isNull())
            nodeId = newNode;
    }

    //=========feat:addObjectTypeArrayVariable============//
    template <typename T, size_t size_array>
    void addObjectTypeArrayVariable(const std::string& n,
                                    const NodeId& parent,
                                    NodeId& nodeId              = NodeId::Null,
                                    NodeContext* context        = nullptr,
                                    const NodeId& requestNodeId = NodeId::Null,  // usually want auto generated ids
                                    bool mandatory              = true)
    {
        T a[size_array]{};
        T type{};
        Variant value(type);
        VariableAttributes var_attr;
        //
        var_attr.setDefault();
        var_attr.setDisplayName(n);
        var_attr.setDescription(n);
        var_attr.get().accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
        Variant variant_array;
        variant_array.setArrayCopy(&a, size_array, value.get().type);
        var_attr.setValue(variant_array);
        //
        QualifiedName qn(_nameSpace, n.c_str());
        //
        NodeId newNode;
        newNode.notNull();
        //
        _server.addVariableNode(requestNodeId,
                                parent,
                                NodeId::HasComponent,
                                qn,
                                NodeId::BaseDataVariableType,
                                var_attr,
                                newNode,
                                context);
        if (mandatory) {
            _server.addReference(newNode, NodeId::HasModellingRule, ExpandedNodeId::ModellingRuleMandatory, true);
        }
        if (!nodeId.isNull())
            nodeId = newNode;
    }

    void addObjectTypeFolder(const std::string& childName,
                             const NodeId& parent,
                             NodeId& nodeId,
                             NodeId& requestNodeId = NodeId::Null,
                             bool mandatory        = true)
    {
        NodeId newNode;
        newNode.notNull();

        _server.addFolder(parent, childName, newNode, requestNodeId);
        if (mandatory) {
            return _server.addReference(newNode,
                                        NodeId::HasModellingRule,
                                        ExpandedNodeId::ModellingRuleMandatory,
                                        true);
        }
        if (!nodeId.isNull())
            nodeId = newNode;
    }

    /*!
        \brief addHistoricalObjectTypeVariable
        \param n
        \param parent
        \param nodeiD
        \param mandatory
        \return
    */
    template <typename T>
    void addHistoricalObjectTypeVariable(const std::string& n,
                                         const NodeId& parent,
                                         NodeId& nodeId        = NodeId::Null,
                                         NodeContext* context  = nullptr,
                                         NodeId& requestNodeId = NodeId::Null,  // usually want auto generated ids
                                         bool mandatory        = true)
    {
        T a{};
        Variant value(a);
        //
        VariableAttributes var_attr;
        var_attr.setDefault();
        var_attr.setDisplayName(n);
        var_attr.setDescription(n);
        var_attr.get().accessLevel =
            UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE | UA_ACCESSLEVELMASK_HISTORYREAD;
        var_attr.setValue(value);
        var_attr.get().dataType    = value.get().type->typeId;
        var_attr.get().historizing = true;

        //
        QualifiedName qn(_nameSpace, n.c_str());
        //
        NodeId newNode;
        newNode.notNull();
        //
        _server.addVariableNode(requestNodeId,
                                parent,
                                NodeId::HasComponent,
                                qn,
                                NodeId::BaseDataVariableType,
                                var_attr,
                                newNode,
                                context);
        if (mandatory) {
            _server.addReference(newNode, NodeId::HasModellingRule, ExpandedNodeId::ModellingRuleMandatory, true);
        }
        if (!nodeId.isNull()) {
            nodeId = newNode;
        }
    }

    /*!
        \brief setMandatory
        \param n1
        \return
    */
    void setMandatory(const NodeId& n1)
    {
        _server.addReference(n1,
                             Open62541::NodeId::HasModellingRule,
                             Open62541::ExpandedNodeId::ModellingRuleMandatory,
                             true);
    }

    /*!
        \brief addDerivedObjectType
        \param server
        \param n
        \param parent
        \param typeId
        \return
    */
    void addDerivedObjectType(const std::string& n,
                              const NodeId& parent,
                              NodeId& typeId,
                              const NodeId& requestNodeId = NodeId::Null,
                              NodeContext* context        = nullptr);
    /*!
        \brief addChildren
        \return
    */
    virtual void addChildren(const NodeId& /*parent*/) {
        throw std::logic_error("addChildren not implemented");
    };
    /*!
        \brief addType
        \param server
        \param baseId
        \return
    */
    virtual void addType(const NodeId& nodeId);  // base node of type
    /*!
        \brief append
        \param parent
        \param nodeId
        \return
    */
    virtual void append(const NodeId& parent,
                        NodeId& nodeId,
                        const NodeId& requestNodeId = NodeId::Null);  // derived type
    /*!
        \brief addInstance
        \param n
        \param parent
        \param nodeId
        \return
    */
    virtual void addInstance(const std::string& n,
                             const NodeId& parent,
                             NodeId& nodeId,
                             const NodeId& requestNodeId = NodeId::Null,
                             NodeContext* context        = nullptr);
};

}  // namespace Open62541
#endif  // SERVEROBJECTTYPE_H
