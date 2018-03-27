
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
#include "open62541objects.h"

// Standard static nodes
Open62541::NodeId   Open62541::NodeId::Objects(0, UA_NS0ID_OBJECTSFOLDER);
Open62541::NodeId   Open62541::NodeId::Server(0, UA_NS0ID_SERVER);
Open62541::NodeId   Open62541::NodeId::Null(0,0);
Open62541::NodeId   Open62541::NodeId::Organizes(0, UA_NS0ID_ORGANIZES);
Open62541::NodeId   Open62541::NodeId::FolderType(0, UA_NS0ID_FOLDERTYPE);
Open62541::NodeId   Open62541::NodeId::HasOrderedComponent(0, UA_NS0ID_HASORDEREDCOMPONENT);
Open62541::NodeId   Open62541::NodeId::BaseObjectType(0, UA_NS0ID_BASEOBJECTTYPE);
Open62541::NodeId   Open62541::NodeId::HasSubType(0, UA_NS0ID_HASSUBTYPE);
Open62541::NodeId   Open62541::NodeId::HasModellingRule(0, UA_NS0ID_HASMODELLINGRULE);
Open62541::NodeId   Open62541::NodeId::ModellingRuleMandatory(0, UA_NS0ID_MODELLINGRULE_MANDATORY);
Open62541::NodeId   Open62541::NodeId::HasComponent(0, UA_NS0ID_HASCOMPONENT);
Open62541::NodeId   Open62541::NodeId::BaseDataVariableType(0,UA_NS0ID_BASEDATAVARIABLETYPE);
Open62541::ExpandedNodeId   Open62541::ExpandedNodeId::ModellingRuleMandatory(UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY));

//
// boost::any to variant conversion
// just basic types
//
/*!
 * \brief Open62541::Variant::fromAny
 * \param a
 */
void Open62541::Variant::fromAny(boost::any &a)
{
    null(); // clear
    const char *t = a.type().name();
    if(!strcmp(t,"string"))
    {
        std::string v = boost::any_cast<std::string>(a);
        UA_String ss;
        ss.length = v.size();
        ss.data = (UA_Byte *)(v.c_str());
        UA_Variant_setScalarCopy((UA_Variant *)ref(), &ss, &UA_TYPES[UA_TYPES_STRING]);
    }
    else if(!strcmp(t,"int"))
    {
        int v = boost::any_cast<int>(a);
        UA_Variant_setScalarCopy((UA_Variant *)ref(), &v, &UA_TYPES[UA_TYPES_INT32]);

    }
    else if(!strcmp(t,"char"))
    {
        short v = short(boost::any_cast<char>(a));
        UA_Variant_setScalarCopy((UA_Variant *)ref(), &v, &UA_TYPES[UA_TYPES_INT16]);
    }
    else if(!strcmp(t,"bool"))
    {
        bool v = boost::any_cast<bool>(a);
        UA_Variant_setScalarCopy((UA_Variant *)ref(), &v, &UA_TYPES[UA_TYPES_BOOLEAN]);
    }
    else if(!strcmp(t,"double"))
    {
        double v = boost::any_cast<double>(a);
        UA_Variant_setScalarCopy((UA_Variant *)ref(), &v, &UA_TYPES[UA_TYPES_DOUBLE]);
    }
    else if(!strcmp(t,"unsigned"))
    {
        unsigned v = boost::any_cast<unsigned>(a);
        UA_Variant_setScalarCopy((UA_Variant *)ref(), &v, &UA_TYPES[UA_TYPES_UINT32]);
    }
    else if(!strcmp(t,"longlong"))
    {
        long long v = boost::any_cast<long long>(a);
        UA_Variant_setScalarCopy((UA_Variant *)ref(), &v, &UA_TYPES[UA_TYPES_INT64]);
    }
    else if(!strcmp(t,"unsignedlonglong"))
    {
        unsigned long long v = boost::any_cast<unsigned long long>(a);
        UA_Variant_setScalarCopy((UA_Variant *)ref(), &v, &UA_TYPES[UA_TYPES_UINT64]);
    }
}



/*!
    \brief toString
    \param n
    \return
*/
std::string Open62541::toString(const UA_NodeId &n) {
    std::string ret = std::to_string(n.namespaceIndex) + ":";

    switch (n.identifierType) {
        case UA_NODEIDTYPE_NUMERIC:
            return ret + std::to_string(n.identifier.numeric);

        case UA_NODEIDTYPE_BYTESTRING:
        case UA_NODEIDTYPE_STRING:
            return ret + std::string((const char *)(n.identifier.string.data), n.identifier.string.length);
        case UA_NODEIDTYPE_GUID: {
            char b[36];
            int l = sprintf(b, "%08X:%04X:%04X[%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X]",
                            n.identifier.guid.data1,
                            n.identifier.guid.data2,
                            n.identifier.guid.data3,
                            n.identifier.guid.data4[0],
                            n.identifier.guid.data4[1],
                            n.identifier.guid.data4[2],
                            n.identifier.guid.data4[3],
                            n.identifier.guid.data4[4],
                            n.identifier.guid.data4[5],
                            n.identifier.guid.data4[6],
                            n.identifier.guid.data4[7]);

            return ret + std::string(b, l);
        }
        default:
            break;
    }
    return std::string("Invalid Node Type");
}


void Open62541::UANodeTree::printNode(UANode *n, std::ostream &os, int level)
{
    if(n)
    {
        std::string indent(level,' ');
        os << indent << n->name();
        os << toString(n->data());
        os << std::endl;
        if(n->children().size() > 0)
        {
            level++;
            for(auto i = n->children().begin(); i != n->children().end(); i++)
            {
                printNode(i->second,os,level); // recurse
            }
        }
    }
}


