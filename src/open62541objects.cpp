
/*
    Copyright (C) 2017 -  B. J. Hill

    This file is part of open62541 C++ classes. open62541 C++ classes are free software: you can
    redistribute it and/or modify it under the terms of the Mozilla Public
    License v2.0 as stated in the LICENSE file provided with open62541.

    open62541 C++ classes are distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
    A PARTICULAR PURPOSE.
*/
#include <open62541cpp/open62541objects.h>
#include <sstream>

// Standard static nodes
Open62541::NodeId Open62541::NodeId::Objects(0, UA_NS0ID_OBJECTSFOLDER);
Open62541::NodeId Open62541::NodeId::Server(0, UA_NS0ID_SERVER);
Open62541::NodeId Open62541::NodeId::Null(0, 0);
Open62541::NodeId Open62541::NodeId::Organizes(0, UA_NS0ID_ORGANIZES);
Open62541::NodeId Open62541::NodeId::FolderType(0, UA_NS0ID_FOLDERTYPE);
Open62541::NodeId Open62541::NodeId::HasOrderedComponent(0, UA_NS0ID_HASORDEREDCOMPONENT);
Open62541::NodeId Open62541::NodeId::BaseObjectType(0, UA_NS0ID_BASEOBJECTTYPE);
Open62541::NodeId Open62541::NodeId::HasSubType(0, UA_NS0ID_HASSUBTYPE);
Open62541::NodeId Open62541::NodeId::HasModellingRule(0, UA_NS0ID_HASMODELLINGRULE);
Open62541::NodeId Open62541::NodeId::ModellingRuleMandatory(0, UA_NS0ID_MODELLINGRULE_MANDATORY);
Open62541::NodeId Open62541::NodeId::HasComponent(0, UA_NS0ID_HASCOMPONENT);
Open62541::NodeId Open62541::NodeId::HasProperty(0, UA_NS0ID_HASPROPERTY);
Open62541::NodeId Open62541::NodeId::BaseDataVariableType(0, UA_NS0ID_BASEDATAVARIABLETYPE);
Open62541::NodeId Open62541::NodeId::HasNotifier(0, UA_NS0ID_HASNOTIFIER);
Open62541::NodeId Open62541::NodeId::BaseEventType(0, UA_NS0ID_BASEEVENTTYPE);

Open62541::ExpandedNodeId Open62541::ExpandedNodeId::ModellingRuleMandatory(
    UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY));

UA_BrowsePathTarget Open62541::BrowsePathResult::nullResult = {UA_EXPANDEDNODEID_NUMERIC(0, 0), 0};

//
// boost::any to variant conversion
// just basic types
//
/*!
    \brief Open62541::Variant::fromAny
    \param a boost::any
*/
void Open62541::Variant::fromAny(boost::any& a)
{
    null();  // clear
    // get the type id as a hash code
    auto t = a.type().hash_code();
    if (t == typeid(std::string).hash_code()) {
        std::string v = boost::any_cast<std::string>(a);
        UA_String ss;
        ss.length = v.size();
        ss.data   = (UA_Byte*)(v.c_str());
        UA_Variant_setScalarCopy((UA_Variant*)ref(), &ss, &UA_TYPES[UA_TYPES_STRING]);
    }
    else if (t == typeid(int).hash_code()) {
        int v = boost::any_cast<int>(a);
        UA_Variant_setScalarCopy((UA_Variant*)ref(), &v, &UA_TYPES[UA_TYPES_INT32]);
    }
    else if (t == typeid(char).hash_code()) {
        short v = short(boost::any_cast<char>(a));
        UA_Variant_setScalarCopy((UA_Variant*)ref(), &v, &UA_TYPES[UA_TYPES_INT16]);
    }
    else if (t == typeid(bool).hash_code()) {
        bool v = boost::any_cast<bool>(a);
        UA_Variant_setScalarCopy((UA_Variant*)ref(), &v, &UA_TYPES[UA_TYPES_BOOLEAN]);
    }
    else if (t == typeid(double).hash_code()) {
        double v = boost::any_cast<double>(a);
        UA_Variant_setScalarCopy((UA_Variant*)ref(), &v, &UA_TYPES[UA_TYPES_DOUBLE]);
    }
    else if (t == typeid(unsigned).hash_code()) {
        unsigned v = boost::any_cast<unsigned>(a);
        UA_Variant_setScalarCopy((UA_Variant*)ref(), &v, &UA_TYPES[UA_TYPES_UINT32]);
    }
    else if (t == typeid(long long).hash_code()) {
        long long v = boost::any_cast<long long>(a);
        UA_Variant_setScalarCopy((UA_Variant*)ref(), &v, &UA_TYPES[UA_TYPES_INT64]);
    }
    else if (t == typeid(unsigned long long).hash_code()) {
        unsigned long long v = boost::any_cast<unsigned long long>(a);
        UA_Variant_setScalarCopy((UA_Variant*)ref(), &v, &UA_TYPES[UA_TYPES_UINT64]);
    }
}

std::string Open62541::variantToString(UA_Variant& v)
{
    std::string ret;
    switch (v.type->typeKind) {
        /**
            Boolean
            ^^^^^^^
        */
        case UA_DATATYPEKIND_BOOLEAN: {
            ret = ((UA_Boolean*)(v.data)) ? "true" : "false";
        } break;

        /**
            SByte
            ^^^^^
        */
        case UA_DATATYPEKIND_SBYTE: {
            int i = *((char*)v.data);
            ret   = std::to_string(i);
        } break;

        /**
            Byte
            ^^^^
        */
        case UA_DATATYPEKIND_BYTE: {
            unsigned i = *((unsigned char*)v.data);
            ret        = std::to_string(i);
        } break;

        /**
            Int16
            ^^^^^
        */
        case UA_DATATYPEKIND_INT16: {
            int16_t i = *((int16_t*)v.data);
            ret       = std::to_string(i);

        } break;

        /**
            UInt16
            ^^^^^^
        */
        case UA_DATATYPEKIND_UINT16: {
            uint16_t i = *((uint16_t*)v.data);
            ret        = std::to_string(i);

        } break;

        /**
            Int32
            ^^^^^
        */
        case UA_DATATYPEKIND_INT32: {
            int32_t i = *((int32_t*)v.data);
            ret       = std::to_string(i);

        } break;

        /**
            UInt32
            ^^^^^^
        */
        case UA_DATATYPEKIND_UINT32: {
            uint32_t i = *((uint32_t*)v.data);
            ret        = std::to_string(i);

        } break;

        /**
            Int64
            ^^^^^
        */
        case UA_DATATYPEKIND_INT64: {
            int64_t i = *((int64_t*)v.data);
            ret       = std::to_string(i);

        } break;

        /**
            UInt64
            ^^^^^^
        */
        case UA_DATATYPEKIND_UINT64: {
            uint32_t i = *((uint32_t*)v.data);
            ret        = std::to_string(i);

        } break;

        /**
            Float
            ^^^^^
        */
        case UA_DATATYPEKIND_FLOAT: {
            float i = *((float*)v.data);
            ret     = std::to_string(i);

        } break;

        /**
            Double
            ^^^^^^
        */
        case UA_DATATYPEKIND_DOUBLE: {
            double i = *((double*)v.data);
            ret      = std::to_string(i);

        } break;

        /**
            String
            ^^^^^^
        */
        case UA_DATATYPEKIND_STRING: {

            UA_String* p = (UA_String*)(v.data);
            ret          = std::string((const char*)p->data, p->length);

        } break;

        /**
            DateTime
            ^^^^^^^^
        */
        case UA_DATATYPEKIND_DATETIME: {
            UA_DateTime* p        = (UA_DateTime*)(v.data);
            UA_DateTimeStruct dts = UA_DateTime_toStruct(*p);
            char b[64];
            int l = sprintf(b,
                            "%02u-%02u-%04u %02u:%02u:%02u.%03u, ",
                            dts.day,
                            dts.month,
                            dts.year,
                            dts.hour,
                            dts.min,
                            dts.sec,
                            dts.milliSec);
            ret   = std::string(b, l);
        } break;

        /**
            ByteString
            ^^^^^^^^^^
        */
        case UA_DATATYPEKIND_BYTESTRING: {
            UA_ByteString* p = (UA_ByteString*)(v.data);
            ret              = std::string((const char*)p->data, p->length);

        } break;

        default:
            break;
    }
    return ret;
}

std::string Open62541::Variant::toString()
{
    return variantToString(*(ref()));
}

/*!
    \brief toString
    \param n
    \return Node in string form
*/
std::string Open62541::toString(const UA_NodeId& n)
{
    std::string ret = std::to_string(n.namespaceIndex) + ":";

    switch (n.identifierType) {
        case UA_NODEIDTYPE_NUMERIC:
            return ret + std::to_string(n.identifier.numeric);

        case UA_NODEIDTYPE_BYTESTRING:
        case UA_NODEIDTYPE_STRING:
            return ret + std::string((const char*)(n.identifier.string.data), n.identifier.string.length);
        case UA_NODEIDTYPE_GUID: {
            char b[45];
            int l = sprintf(b,
                            "%08X:%04X:%04X[%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X]",
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

/*!
    \brief Open62541::UANodeTree::printNode
    \param n
    \param os
    \param level
*/
void Open62541::UANodeTree::printNode(UANode* n, std::ostream& os, int level)
{
    if (n) {
        std::string indent(level, ' ');
        os << indent << n->name();
        os << " : " << toString(n->data());
        os << std::endl;
        if (n->children().size() > 0) {
            level++;
            for (auto i = n->children().begin(); i != n->children().end(); i++) {
                printNode(i->second, os, level);  // recurse
            }
        }
    }
}

/*!
    \brief Open62541::BrowserBase::browseIter
    \param childId
    \param isInverse
    \param referenceTypeId
    \param handle
    \return status
*/
UA_StatusCode Open62541::BrowserBase::browseIter(UA_NodeId childId,
                                                 UA_Boolean isInverse,
                                                 UA_NodeId referenceTypeId,
                                                 void* handle)
{
    // node iterator for browsing
    if (isInverse) return UA_STATUSCODE_GOOD;  // TO DO what does this do?
    Open62541::BrowserBase* p = (Open62541::BrowserBase*)handle;
    if (p) {
        p->process(childId, referenceTypeId);  // process record
    }
    return UA_STATUSCODE_GOOD;
}

/*!
    \brief print
    \param os
*/
void Open62541::BrowserBase::print(std::ostream& os)
{
    for (BrowseItem& i : _list) {
        std::string s;
        int j;
        NodeId n;
        n = i.childId;
        if (browseName(n, s, j)) {
            os << toString(i.childId) << " ns:" << i.nameSpace << ": " << i.name
               << " Ref:" << toString(i.referenceTypeId) << std::endl;
        }
    }
}
/*!
    \brief find
    \param s
    \return iterator to found item or list().end()
*/
Open62541::BrowseList::iterator Open62541::BrowserBase::find(const std::string& s)
{
    BrowseList::iterator i = _list.begin();
    for (i = _list.begin(); i != _list.end(); i++) {
        if ((*i).name == s) break;
    }
    return i;
}

/*!
    \brief process
    \param childId
    \param referenceTypeId
*/
void Open62541::BrowserBase::process(UA_NodeId childId, UA_NodeId referenceTypeId)
{
    std::string s;
    int i;
    NodeId n;
    n = childId;
    if (browseName(n, s, i)) {
        _list.push_back(BrowseItem(s, i, childId, referenceTypeId));
    }
}

/*!
    \brief printTimestamp
    \param name
    \param date
*/
std::string Open62541::timestampToString(UA_DateTime date)
{
    UA_DateTimeStruct dts = UA_DateTime_toStruct(date);
    char b[64];
    int l = sprintf(b,
                    "%02u-%02u-%04u %02u:%02u:%02u.%03u, ",
                    dts.day,
                    dts.month,
                    dts.year,
                    dts.hour,
                    dts.min,
                    dts.sec,
                    dts.milliSec);
    return std::string(b, l);
}

/*!
    \brief dataValueToString
    \param value
*/

std::string Open62541::dataValueToString(UA_DataValue* value)
{
    std::stringstream os;
    /* Print status and timestamps */
    os << "ServerTime:" << timestampToString(value->serverTimestamp) << " ";
    os << "SourceTime:" << timestampToString(value->sourceTimestamp) << " ";
    os << "Status:" << std::hex << value->status << " ";
    os << "Value:" << variantToString(value->value);
    return os.str();
}
