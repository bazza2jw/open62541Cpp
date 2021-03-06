/*
    Copyright (C) 2017 -  B. J. Hill

    This file is part of open62541 C++ classes. open62541 C++ classes are free software: you can
    redistribute it and/or modify it under the terms of the Mozilla Public
    License v2.0 as stated in the LICENSE file provided with open62541.

    open62541 C++ classes are distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
    A PARTICULAR PURPOSE.
*/
#ifndef OPEN62541OBJECTS_H
#define OPEN62541OBJECTS_H

#if defined(_MSC_VER)
// get rid of template not exported warnings - warning is meaningless as it cannot be fixed
#pragma warning(disable:4251)
#endif
//
#ifdef UA_ENABLE_AMALGAMATION
#include "open62541.h"
#else
#include "open62541/config.h"
#include "open62541/statuscodes.h"
#include "open62541/nodeids.h"
#include "open62541/common.h"
#include "open62541/types.h"
#include "open62541/util.h"
#include "open62541/client.h"
#include "open62541/server.h"
#include "open62541/architecture_definitions.h"
#include "open62541/server_pubsub.h"
#include "open62541/types_generated.h"
#include "open62541/network_tcp.h"
#include "open62541/client_config_default.h"
#include "open62541/client_highlevel_async.h"
#include "open62541/types_generated_handling.h"
#include "open62541/architecture_functions.h"
#include "open62541/posix/ua_architecture.h"
#include "open62541/server_config_default.h"
#include "open62541/client_subscriptions.h"
#include "open62541/client_highlevel.h"
#include "open62541/plugin/historydatabase.h"
#include "open62541/plugin/log_syslog.h"
#include "open62541/plugin/network.h"
#include "open62541/plugin/historydata/history_database_default.h"
#include "open62541/plugin/historydata/history_data_backend.h"
#include "open62541/plugin/historydata/history_data_gathering.h"
#include "open62541/plugin/historydata/history_data_gathering_default.h"
#include "open62541/plugin/historydata/history_data_backend_memory.h"
#include "open62541/plugin/log.h"
#include "open62541/plugin/nodestore.h"
#include "open62541/plugin/pki.h"
#include "open62541/plugin/pubsub.h"
#include "open62541/plugin/nodestore_default.h"
#include "open62541/plugin/log_stdout.h"
#include "open62541/plugin/securitypolicy.h"
#include "open62541/plugin/accesscontrol.h"
#include "open62541/plugin/accesscontrol_default.h"
#include "open62541/plugin/pki_default.h"
#include "open62541/plugin/securitypolicy_default.h"
#endif
//
#if UA_MULTITHREADING >= 100
// Sleep is function call in wxWidgets
#include <pthread.h>
#undef Sleep
#endif
//
#include "open62541cpp_trace.h"
//
#include <string>
#include <stdint.h>
#if defined(__GNUC__)
#include <error.h>
#endif
//
#include <map>
#include <vector>
#include <stdexcept>
#include <memory>
#include <iostream>
#include <functional>
#include <typeinfo>
#include "propertytree.h"
#include <boost/any.hpp>
//
// Open 62541 has quasi new-delete and copy operators for each object type
// define wrappers for Open 62541 objects
//
// With Microsoft Windows watch out for class export problems
// Sometimes templates have to include UA_EXPORT othertimes not
// If the template is typedef'ed do not export
// If the template is the base of a class it is exported
//
namespace Open62541 {
//
// Base wrapper for most C open62541 object types
// use unique_ptr
//
template<typename T> class UA_EXPORT TypeBase {
protected:
    std::unique_ptr<T> _d; // shared pointer - there is no copy on change
public:
    TypeBase(T *t) : _d(t) {}
    T &get() const {
        return *(_d.get());
    }
    // Reference and pointer for parameter passing
    operator T &() const {
        return get();
    }

    operator T *() const {
        return _d.get();
    }
    const T *constRef() const {
        return _d.get();
    }

    T   *ref() const {
        return _d.get();
    }

};
//
// Repeated for each type but cannot use C++ templates because we must also wrap the C function calls for each type
// initialisation implies shallow copy and so takes ownership ,  assignment is deep copy so source is not owned
//
// trace the object create and delete - only for detailed testing / debugging
#define UA_STRINGIFY(a) __UA_STRINGIFY(a)
#define __UA_STRINGIFY(a) #a

#ifdef UA_TRACE_OBJ
#define UA_TRC(s) std::cout << s << std::endl;
#else
#define UA_TRC(s)
#endif
//
// copies are all deep copies
//
#define UA_TYPE_BASE(C,T)\
    C() :  TypeBase(T##_new()) {T##_init(_d.get()); UA_TRC("Construct:" << UA_STRINGIFY(C))}\
    C(const T &t) :  TypeBase(T##_new()){assignFrom(t);UA_TRC("Construct (" << UA_STRINGIFY(T) << ")")}\
    ~C(){UA_TRC("Delete:" << UA_STRINGIFY(C)); if(_d) T##_clear(_d.get());}\
    C(const C & n) :  TypeBase(T##_new())  { T##_copy(n._d.get(),_d.get()); UA_TRC("Copy Construct:" << UA_STRINGIFY(C))}\
    C & operator = ( const C &n) {UA_TRC("Assign:" << UA_STRINGIFY(C));null(); T##_copy(n._d.get(),_d.get()); return *this;}\
    void null() {if(_d){UA_TRC("Delete(in null):" << UA_STRINGIFY(C));T##_clear(_d.get());} _d.reset(T##_new());T##_init(_d.get());}\
    void assignTo(T &v){ T##_copy(_d.get(),&v);}\
    void assignFrom(const T &v){ T##_copy(&v,_d.get());}

/*!
 * \brief The String class
 */
class String {
    UA_String _s;
public:
    String(const std::string &s)
    {
        _s = UA_String_fromChars(s.c_str());
    }

    String(const String &s)
    {
        UA_String_clear(&_s);
        UA_String_copy(&s._s,&_s);
    }

    String(const UA_String &s)
    {
        UA_String_clear(&_s);
        UA_String_copy(&s,&_s);
    }

    ~String()
    {
        UA_String_clear(&_s);
    }

    operator const UA_String & () {
        return _s;
    }
    operator const UA_String * () {
        return &_s;
    }
    operator UA_String * () {
        return &_s;
    }


    String & operator = (const String &s)
    {
        UA_String_clear(&_s);
        UA_String_copy(&s._s,&_s);
        return *this;
    }

    String & operator = (const UA_String &s)
    {
        UA_String_clear(&_s);
        UA_String_copy(&s,&_s);
        return *this;
    }

    std::string toStdString()
    {
        return std::string((char *)(_s.data),_s.length);
    }


};


class ByteString {
    UA_ByteString _s;
public:
    ByteString(const std::string &s)
    {
        _s = UA_BYTESTRING_ALLOC(s.c_str());
    }

    ByteString(const ByteString &s)
    {
        UA_ByteString_clear(&_s);
        UA_ByteString_copy(&s._s,&_s);
    }

    ByteString(const UA_ByteString &s)
    {
        UA_ByteString_clear(&_s);
        UA_ByteString_copy(&s,&_s);
    }

    ~ByteString()
    {
        UA_ByteString_clear(&_s);
    }

    operator const UA_ByteString & () {
        return _s;
    }
    operator const UA_ByteString * () {
        return &_s;
    }
    operator UA_ByteString * () {
        return &_s;
    }


    ByteString & operator = (const ByteString &s)
    {
        UA_ByteString_clear(&_s);
        UA_ByteString_copy(&s._s,&_s);
        return *this;
    }

    ByteString & operator = (const UA_ByteString &s)
    {
        UA_ByteString_clear(&_s);
        UA_ByteString_copy(&s,&_s);
        return *this;
    }

    std::string toStdString()
    {
        return std::string((char *)(_s.data),_s.length);
    }


};



#define UA_TYPE_DEF(T) UA_TYPE_BASE(T,UA_##T)


template <typename T, const UA_UInt32 I>
/*!
       \brief The Array class
       This is for allocated arrays of UA_ objects
       simple lifecycle management.
       Uses UA_array_new and UA_array_delete
       rather than new and delete
       Also deals with arrays returned from UA_ functions
       The optional initFunc and clearFunc parameters are the UA initialise and clear functions for the structure
*/



class Array {
    T *_data = nullptr;
    size_t _length = 0;

public:
    Array() { release();}
    Array(T *data, size_t len)
        : _data(data), _length(len) {
        // shallow copy
    }

    Array(size_t n) {
        if(n > 0)
        {
            allocate(n);
            prepare();
        }
    }

    virtual ~Array() {
        clear();
    }

    /*!
     * \brief dataType
     * \return
     */
    const UA_DataType * dataType() { return  &UA_TYPES[I];}
    /*!
        \brief allocate
        \param len
    */
    void allocate(size_t len) {
        clear();
        _data = (T *)(UA_Array_new(len, dataType()));
        _length = len;
    }

    /*!
        \brief release
        detach and transfer ownership to the caller - no longer managed
    */
    void release() {
        _length = 0;
        _data = nullptr;
    }

    virtual void clearFunc(T *){}
    /*!
        \brief clear
    */
    void clear() {
        if (_length && _data) {
            T * p = _data;
            for(size_t i = 0; i < _length; i++, p++)
            {
                clearFunc(p);
            }
            UA_Array_delete(_data, _length, dataType());
        }
        _length = 0;
        _data = nullptr;
    }

    /*!
        \brief at
        \return
    */
    T &at(size_t i) const {
        if (!_data || (i >= _length)) throw std::exception();
        return _data[i];
    }

    /*!
        \brief setList
        \param len
        \param data
    */
    void setList(size_t len, T *data) {
        clear();
        _length = len;
        _data = data;
    }

    // Accessors
    size_t length() const {
        return _length;
    }
    T *data() const {
        return _data;
    }
    //
    size_t *lengthRef()  {
        return &_length;
    }
    T **dataRef()  {
        return &_data;
    }
    //
    operator T *() {
        return _data;
    }
    //
    virtual void initFunc(T *) {}
    void prepare()
    {
        if(_length > 0)
        {
            T * p = _data;
            for(size_t i = 0; i < _length; i++, p++)
            {
                initFunc(p);
            }
        }
    }

};


//
// typedef basic array types
// macro to define the init and clear functions for the complex objects
#define ARRAY_INIT_CLEAR(T) void clearFunc(T * p) { T##_clear(p);} void initFunc(T * p) { T##_init(p);}

typedef std::vector<std::string> StdStringArray;

// Generate an array declaration with the type specific constructors and destructors - has to the use the UA versions
#define TYPEDEF_ARRAY(T,I) \
typedef Array<UA_##T, I> T##Array_Base;\
class T##Array : public  T##Array_Base\
{\
public:\
    T##Array() {}\
    T##Array(UA_##T *data, size_t len):T##Array_Base(data,len){}\
    T##Array(size_t n):T##Array_Base(n) {}\
    ARRAY_INIT_CLEAR(UA_##T)\
};
TYPEDEF_ARRAY(String, UA_TYPES_STRING)
TYPEDEF_ARRAY(NodeId, UA_TYPES_NODEID)





// non-heap allocation - no delete
/*!
    \brief toUA_String
    \param s
    \return
*/
inline UA_String  toUA_String(const std::string &s) {
    UA_String r;
    r.length = s.size();
    r.data = (UA_Byte *)(s.c_str());
    return r;
}


/*!
    \brief fromStdString
    \param s
    \param r
*/
inline void fromStdString(const std::string &s, UA_String &r) {
    UA_String_clear(&r);
    r = UA_STRING_ALLOC(s.c_str());
}

/*!
 * \brief fromByteString
 * \param b
 * \return
 */
inline std::string  fromByteString(UA_ByteString &b)
{
    std::string s((const char *)b.data,b.length);
    return s;
}

/*!
    \brief printLastError
    \param c
*/
inline void printLastError(UA_StatusCode c, std::iostream &os) {
    os << UA_StatusCode_name(c) ;
}

/*!
    \brief toString
    \param c
    \return
*/
inline std::string toString(UA_StatusCode c) {
    return std::string(UA_StatusCode_name(c));
}

// Prints status only if not Good
#define UAPRINTLASTERROR(c) {if(c != UA_STATUSCODE_GOOD) std::cerr << __FUNCTION__ << ":" << __LINE__ << ":" << UA_StatusCode_name(c) << std::endl;}
/*!
       \brief The UsernamePasswordLogin class
*/
class UA_EXPORT UsernamePasswordLogin : public TypeBase<UA_UsernamePasswordLogin> {
public:

    UsernamePasswordLogin(const std::string &u = "", const std::string &p = "")  :  TypeBase(new UA_UsernamePasswordLogin()) {
        UA_String_init(&ref()->username);
        UA_String_init(&ref()->password);
        setUserName(u);
        setPassword(p);
    }

    ~UsernamePasswordLogin() {
        UA_String_clear(&ref()->username);
        UA_String_clear(&ref()->password);
    }

    /*!
        \brief setUserName
        \param s
    */
    void setUserName(const std::string &s) {
        fromStdString(s, ref()->username);
    }
    /*!
        \brief setPassword
        \param s
    */
    void setPassword(const std::string &s) {
        fromStdString(s, ref()->password);
    }

};
/*!
    \brief The ObjectAttributes class
*/
class  UA_EXPORT  ObjectAttributes : public TypeBase<UA_ObjectAttributes> {
public:
    UA_TYPE_DEF(ObjectAttributes)
    void setDefault() {
        *this = UA_ObjectAttributes_default;
    }

    void setDisplayName(const std::string &s) {
        get().displayName = UA_LOCALIZEDTEXT_ALLOC("en_US", s.c_str());
    }
    void setDescription(const std::string &s) {
        get().description = UA_LOCALIZEDTEXT_ALLOC("en_US", s.c_str());
    }
    void setSpecifiedAttributes(UA_UInt32 m) {
        get().specifiedAttributes = m;
    }
    void setWriteMask(UA_UInt32 m) {
        get().writeMask = m;
    }
    void setUserWriteMask(UA_UInt32 m) {
        get().userWriteMask = m;
    }
    void setEventNotifier(unsigned m) {
        get().eventNotifier = (UA_Byte)m;
    }
};

/*!
    \brief The ObjectTypeAttributes class
*/
class  UA_EXPORT  ObjectTypeAttributes : public TypeBase<UA_ObjectTypeAttributes> {
public:
    UA_TYPE_DEF(ObjectTypeAttributes)
    void setDefault() {
        *this = UA_ObjectTypeAttributes_default;
    }

    void setDisplayName(const std::string &s) {
        get().displayName = UA_LOCALIZEDTEXT_ALLOC("en_US", s.c_str());
    }
    void setDescription(const std::string &s) {
        get().description = UA_LOCALIZEDTEXT_ALLOC("en_US", s.c_str());
    }
    void setSpecifiedAttributes(UA_UInt32 m) {
        get().specifiedAttributes = m;
    }
    void setWriteMask(UA_UInt32 m) {
        get().writeMask = m;
    }
    void setUserWriteMask(UA_UInt32 m) {
        get().userWriteMask = m;
    }
    void setIsAbstract(bool f) {
        get().isAbstract = f;
    }

};

/*!
    \brief The NodeClass class
*/
typedef UA_NodeClass NodeClass;
/*!
       \brief The NodeId class
*/
class  UA_EXPORT  NodeId : public TypeBase<UA_NodeId> {
public:

    // Common constant nodes
    static NodeId  Null;
    static NodeId  Objects;
    static NodeId  Server;
    static NodeId  Organizes;
    static NodeId  FolderType;
    static NodeId  HasOrderedComponent;
    static NodeId  BaseObjectType;
    static NodeId  HasSubType;
    static NodeId  HasModellingRule;
    static NodeId  ModellingRuleMandatory;
    static NodeId  HasComponent;
    static NodeId  BaseDataVariableType;
    static NodeId  HasProperty;
    static NodeId  HasNotifier;
    static NodeId  BaseEventType;

    //
    UA_TYPE_DEF(NodeId)
    //
    // null
    //
    bool isNull() const {
        return UA_NodeId_isNull(constRef());
    }

    // equality
    bool operator == (const NodeId &n) {
        return UA_NodeId_equal(_d.get(), n._d.get());
    }
    /* Returns a non-cryptographic hash for the NodeId */
    unsigned hash() const {
        return UA_NodeId_hash(constRef());
    }

    // human friendly id string
    NodeId(const char *id) : TypeBase(UA_NodeId_new())
    {
        *(_d.get()) = UA_NODEID(id); // parses the string to a node id
    }
    // Specialised constructors
    NodeId(unsigned index, unsigned id) : TypeBase(UA_NodeId_new()) {
        *(_d.get()) = UA_NODEID_NUMERIC(UA_UInt16(index), id);
    }

    NodeId(unsigned index, const std::string &id) : TypeBase(UA_NodeId_new()) {
        *(_d.get()) = UA_NODEID_STRING_ALLOC(UA_UInt16(index), id.c_str());
    }


    NodeId(unsigned index, UA_Guid guid) : TypeBase(UA_NodeId_new()) {
        *(_d.get()) = UA_NODEID_GUID(UA_UInt16(index), guid);
    }
    //
    // accessors
    int nameSpaceIndex() const {
        return constRef()->namespaceIndex;
    }
    UA_NodeIdType identifierType() const {
        return constRef()->identifierType;
    }
    //
    NodeId &notNull() { // makes a node not null so new nodes are returned to references
        null(); // clear anything beforehand
        *(_d.get()) = UA_NODEID_NUMERIC(1, 0); // force a node not to be null
        return *this;
    }

    bool toString(std::string &s) const // C library version of nodeid to string
    {
        UA_String o;
        UA_NodeId_print(this->constRef(), &o);
        s = std::string((char *)o.data,o.length);
        UA_String_clear(&o);
        return true;
    }

    UA_UInt32 numeric() const {
        return constRef()->identifier.numeric;
    }
    const UA_String  &   string() {
        return constRef()->identifier.string;
    }
    const UA_Guid    &   guid() {
        return constRef()->identifier.guid;
    }
    const UA_ByteString & byteString() {
        return constRef()->identifier.byteString;
    }

    const UA_DataType  *  findDataType() const { return  UA_findDataType(constRef());}


};

/*!
    \brief toString
    \param n
    \return node identifier as string
*/
UA_EXPORT  std::string toString(const UA_NodeId &n);


// use for browse lists
/*!
    \brief The UANodeIdList class
*/
class  UA_EXPORT   UANodeIdList : public std::vector<UA_NodeId> {
public:
    UANodeIdList() {}
    virtual ~UANodeIdList() {
        for (int i = 0 ; i < int(size()); i++) {
            UA_NodeId_clear(&(at(i))); // delete members
        }
    }
    void put(UA_NodeId &n) {
        UA_NodeId i; // deep copy
        UA_NodeId_init(&i);
        UA_NodeId_copy(&n, &i);
        push_back(i);
    }
};

/*!
    \brief The NodeIdMap class
*/
class  UA_EXPORT  NodeIdMap : public std::map<std::string, UA_NodeId> {
public:
    NodeIdMap() {} // set of nodes not in a tree
    virtual ~NodeIdMap() {
        // delete node data
        for (auto i = begin(); i != end(); i++) {
            UA_NodeId &n = i->second;
            UA_NodeId_clear(&n);
        }
        clear();
    }
    void put(UA_NodeId &n) {
        UA_NodeId i; // deep copy
        UA_NodeId_init(&i);
        UA_NodeId_copy(&n, &i);
        const std::string s = Open62541::toString(i);
        insert(std::pair<std::string, UA_NodeId>(s, i));
    }
};


/*!
    \brief The ExpandedNodeId class
*/
class  UA_EXPORT  ExpandedNodeId : public TypeBase<UA_ExpandedNodeId> {
public:
    UA_TYPE_DEF(ExpandedNodeId)
    static ExpandedNodeId  ModellingRuleMandatory;

    ExpandedNodeId(const std::string namespaceUri, UA_NodeId &node, int serverIndex) : TypeBase(UA_ExpandedNodeId_new()) {
        ref()->namespaceUri = UA_STRING_ALLOC(namespaceUri.c_str());
        UA_NodeId_copy(&get().nodeId, &node); // deep copy across
        ref()->serverIndex = serverIndex;
    }


    bool toString(std::string &s) const // C library version of nodeid to string
    {
        UA_String o;
        UA_ExpandedNodeId_print(this->constRef(), &o);
        s = std::string((char *)o.data,o.length);
        UA_String_clear(&o);
        return true;
    }

    /* Parse the ExpandedNodeId format defined in Part 6, 5.3.1.11:
     *
     *   svr=<serverindex>;ns=<namespaceindex>;<type>=<value>
     *     or
     *   svr=<serverindex>;nsu=<uri>;<type>=<value>
     *
     * The definitions for svr, ns and nsu can be omitted and will be set to zero /
     * the empty string.*/
    bool parse(const std::string &s)
    {
        UA_String str;
        str.data = (UA_Byte *)s.c_str();
        str.length = s.length();
        return UA_ExpandedNodeId_parse(ref(),str) == UA_STATUSCODE_GOOD;
    }

    ExpandedNodeId(const char *chars) : TypeBase(UA_ExpandedNodeId_new())
    {
        get() =  UA_EXPANDEDNODEID(chars);
    }

    /** The following functions are shorthand for creating ExpandedNodeIds. */
    ExpandedNodeId(UA_UInt16 nsIndex, UA_UInt32 identifier) : TypeBase(UA_ExpandedNodeId_new())
    {
        get() = UA_EXPANDEDNODEID_NUMERIC(nsIndex, identifier);
    }

    ExpandedNodeId(UA_UInt16 nsIndex,const std::string &chars) : TypeBase(UA_ExpandedNodeId_new())
    {
        get() = UA_EXPANDEDNODEID_STRING_ALLOC(nsIndex, chars.c_str());
    }


    ExpandedNodeId(UA_UInt16 nsIndex, char *chars) : TypeBase(UA_ExpandedNodeId_new())
    {
        get() = UA_EXPANDEDNODEID_STRING( nsIndex, chars);
    }

    ExpandedNodeId(UA_UInt16 nsIndex,const char *chars) : TypeBase(UA_ExpandedNodeId_new())
    {
        get() = UA_EXPANDEDNODEID_STRING_ALLOC(nsIndex, chars);
    }

    ExpandedNodeId(UA_UInt16 nsIndex, UA_Guid guid) : TypeBase(UA_ExpandedNodeId_new())
    {
        get() = UA_EXPANDEDNODEID_STRING_GUID(nsIndex,  guid);
    }

    ExpandedNodeId(UA_UInt16 nsIndex, unsigned char *chars) : TypeBase(UA_ExpandedNodeId_new())
    {
        get() = UA_EXPANDEDNODEID_BYTESTRING(nsIndex, (char *)chars);
    }

    ExpandedNodeId(UA_UInt16 nsIndex,const unsigned char *chars) : TypeBase(UA_ExpandedNodeId_new())
    {
        get() = UA_EXPANDEDNODEID_BYTESTRING_ALLOC(nsIndex, (char *)chars);
    }

    /* Does the ExpandedNodeId point to a local node? That is, are namespaceUri and
     * serverIndex empty? */
    bool isLocal() const
    {
        return UA_ExpandedNodeId_isLocal(constRef()) == UA_TRUE;
    }

    /* Total ordering of ExpandedNodeId */
    static UA_Order order(const UA_ExpandedNodeId *n1, const UA_ExpandedNodeId *n2)
    {
        return UA_ExpandedNodeId_order(n1, n2);
    }
    bool operator == (const ExpandedNodeId &e )
    {
        return UA_ExpandedNodeId_order(constRef(), e.constRef()) == UA_ORDER_EQ;
    }

    bool operator > (const ExpandedNodeId &e)
    {
        return UA_ExpandedNodeId_order(constRef(), e.constRef()) == UA_ORDER_MORE;
    }

    bool operator < (const ExpandedNodeId &e)
    {
        return UA_ExpandedNodeId_order(constRef(), e.constRef()) == UA_ORDER_LESS;
    }

    /* Returns a non-cryptographic hash for ExpandedNodeId. The hash of an
     * ExpandedNodeId is identical to the hash of the embedded (simple) NodeId if
     * the ServerIndex is zero and no NamespaceUri is set. */
    UA_UInt32 hash() const {
        return  UA_ExpandedNodeId_hash(constRef());
    }
    UA_NodeId & nodeId () {
        return ref()->nodeId;
    }
    UA_String & namespaceUri() {
        return ref()->namespaceUri;
    }
    UA_UInt32 serverIndex() {
        return ref()->serverIndex;
    }
};

/*!
 * \brief The BrowsePathResult class
 */
class UA_EXPORT BrowsePathResult : public TypeBase<UA_BrowsePathResult> {
    static UA_BrowsePathTarget nullResult;
public:
    UA_TYPE_DEF(BrowsePathResult)
    UA_StatusCode statusCode() const
    {
        return ref()->statusCode;
    }
    size_t targetsSize() const
    {
        return ref()->targetsSize;
    }
    UA_BrowsePathTarget targets(size_t i)
    {
        if(i < ref()->targetsSize)  return ref()->targets[i];
        return nullResult;
    }
};

/*!
    \brief toString
    \param r
    \return UA_String as std::string
*/
inline std::string toString(UA_String &r) {
    std::string s((const char *)(r.data), r.length);
    return s;
}


/*!
    \brief The uaVariant class
*/
//
// Managed variant type
//
// Memory Leak Risk - TODO Check this
//
std::string variantToString(UA_Variant &v);
class  UA_EXPORT  Variant  : public TypeBase<UA_Variant> {
public:
    // It would be nice to template but ...
    UA_TYPE_DEF(Variant)

    //
    // Construct Variant from ...
    // TO DO add array handling
    /*!
        \brief uaVariant
        \param v
    */
    Variant(const std::string &v) : TypeBase(UA_Variant_new()) {
        UA_String ss;
        ss.length = v.size();
        ss.data = (UA_Byte *)(v.c_str());
        UA_Variant_setScalarCopy((UA_Variant *)ref(), &ss, &UA_TYPES[UA_TYPES_STRING]);
    }

    Variant(const char *locale, const char *text): TypeBase(UA_Variant_new()) {
        UA_LocalizedText t =  UA_LOCALIZEDTEXT((char *)locale,(char *)text); // just builds does not allocate
        UA_Variant_setScalarCopy((UA_Variant *)ref(), &t, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
    }

    Variant(UA_UInt64 v) : TypeBase(UA_Variant_new()) {
        UA_Variant_setScalarCopy((UA_Variant *)ref(), &v, &UA_TYPES[UA_TYPES_UINT64]);
    }

    Variant(UA_UInt16 v)  : TypeBase(UA_Variant_new()) {
        UA_Variant_setScalarCopy((UA_Variant *)ref(), &v, &UA_TYPES[UA_TYPES_UINT16]);
    }

    Variant(UA_String &v) : TypeBase(UA_Variant_new()) {
        UA_Variant_setScalarCopy((UA_Variant *)ref(), &v, &UA_TYPES[UA_TYPES_STRING]);
    }

    Variant(const char *v) : TypeBase(UA_Variant_new()) {
        UA_String ss = UA_STRING((char *)v);
        UA_Variant_setScalarCopy((UA_Variant *)ref(), &ss, &UA_TYPES[UA_TYPES_STRING]);
    }

    /*!
        \brief uaVariant
        \param a
    */
    Variant(int a) : TypeBase(UA_Variant_new()) {
        UA_Variant_setScalarCopy((UA_Variant *)ref(), &a, &UA_TYPES[UA_TYPES_INT32]);
    }

    /*!
        \brief uaVariant
        \param a
    */
    Variant(unsigned a) : TypeBase(UA_Variant_new()) {
        UA_Variant_setScalarCopy((UA_Variant *)ref(), &a, &UA_TYPES[UA_TYPES_UINT32]);
    }

    /*!
        \brief uaVariant
        \param a
    */
    Variant(double a) : TypeBase(UA_Variant_new()) {
        UA_Variant_setScalarCopy((UA_Variant *)ref(), &a, &UA_TYPES[UA_TYPES_DOUBLE]);
    }

    /*!
        \brief uaVariant
        \param a
    */
    Variant(bool a) : TypeBase(UA_Variant_new()) {
        UA_Variant_setScalarCopy((UA_Variant *)ref(), &a, &UA_TYPES[UA_TYPES_BOOLEAN]);
    }

    /*!
        \brief Variant
        \param t
    */
    Variant(UA_DateTime t): TypeBase(UA_Variant_new()) {
        UA_Variant_setScalarCopy((UA_Variant *)ref(), &t, &UA_TYPES[UA_TYPES_DATETIME]);
    }

    /*!
        cast to a type supported by UA
    */
    template<typename T> T value() {
        if (!UA_Variant_isEmpty((UA_Variant *)ref())) {
            return *((T *)ref()->data); // cast to a value - to do Type checking needed
        }
        return T();
    }

    /*!
        \brief empty
        \return
    */
    bool empty() {
        return UA_Variant_isEmpty(ref());
    }
    /*!
        \brief clear
    */
    void clear() {
        if (!UA_Variant_isEmpty(ref())) {
            if (get().storageType == UA_VARIANT_DATA) {
                UA_Variant_clear((UA_Variant *)ref());
            }
        }
    }
    //
    //
    bool isScalar() const { return   UA_Variant_isScalar(constRef());}
    bool hasScalarType(const UA_DataType *type) { return   UA_Variant_hasScalarType(constRef(), type);}
    bool hasArrayType(const UA_DataType *type){ return  UA_Variant_hasArrayType(constRef(),type);}
    void setScalar(void * UA_RESTRICT p,  const UA_DataType *type) {  UA_Variant_setScalar(ref(), p,type);}
    bool  setScalarCopy(const void *p, const UA_DataType *type){ return   UA_Variant_setScalarCopy(ref(),p,type) == UA_STATUSCODE_GOOD;}
    void setArray(void * UA_RESTRICT array, size_t arraySize, const UA_DataType *type) { UA_Variant_setArray(ref(),  array, arraySize, type); }
    bool setArrayCopy(const void *array, size_t arraySize, const UA_DataType *type) { return  UA_Variant_setArrayCopy(ref(), array,arraySize, type) == UA_STATUSCODE_GOOD;}
    bool copyRange(Variant &src, const UA_NumericRange range) { return  UA_Variant_copyRange(src.ref(), ref(), range) == UA_STATUSCODE_GOOD;} // copy to this variant
    static bool copyRange(Variant &src, Variant &dst, const UA_NumericRange range) { return  UA_Variant_copyRange(src.ref(), dst.ref(), range) == UA_STATUSCODE_GOOD;}
    bool setRange( void * UA_RESTRICT array,  size_t arraySize, const UA_NumericRange range){ return  UA_Variant_setRange(ref(), array, arraySize, range) == UA_STATUSCODE_GOOD;}
    //
    const UA_DataType * dataType() { return ref()->type;}
    bool isType(const UA_DataType *i ) { return ref()->type == i;} // compare types
    bool isType(UA_Int32 i ) { return ref()->type == &UA_TYPES[i];} // compare types
    //
    // convert from an any to Variant
    // limit to basic types
    void fromAny(boost::any &a);
    /*!
     * \brief toString
     * \return variant in string form
     */
    std::string toString();
};

TYPEDEF_ARRAY(Variant,UA_TYPES_VARIANT)

/*!
    \brief The QualifiedName class
*/
class  UA_EXPORT  QualifiedName  : public TypeBase<UA_QualifiedName> {
public:
    UA_TYPE_DEF(QualifiedName)
    QualifiedName(int ns, const char *s) : TypeBase(UA_QualifiedName_new()) {
        *(_d.get()) = UA_QUALIFIEDNAME_ALLOC(ns, s);
    }
    QualifiedName(int ns, const std::string &s) : TypeBase(UA_QualifiedName_new()) {
        *(_d.get()) = UA_QUALIFIEDNAME_ALLOC(ns, s.c_str());
    }

    UA_UInt16 namespaceIndex() {
        return ref()->namespaceIndex;
    }
    UA_String &name() {
        return ref()->name;
    }
};


//
/*!
    \brief Path
*/
typedef std::vector<std::string> Path;

/*!
    \brief The BrowseItem struct
*/
struct  UA_EXPORT  BrowseItem {
    std::string name; // the browse name
    int nameSpace = 0;
    UA_NodeId childId; // not managed - shallow copy
    UA_NodeId referenceTypeId; // not managed - shallow copy
    BrowseItem(const std::string &n,
               int ns,
               UA_NodeId c,
               UA_NodeId r)
        : name(n), nameSpace(ns),
          childId(c),
          referenceTypeId(r) {}
    BrowseItem(const BrowseItem &b) :
        name(b.name),
        nameSpace(b.nameSpace) {
        childId = b.childId;
        referenceTypeId = b.referenceTypeId;
    }
};



//
// Helper containers
//
class  UA_EXPORT  ArgumentList : public std::vector<UA_Argument> {
public:
    //
    // use constant strings for argument names - else memory leak
    //
    void addScalarArgument(const char *s, int type) {
        UA_Argument a;
        UA_Argument_init(&a);
        a.description = UA_LOCALIZEDTEXT((char *)"en_US", (char *)s);
        a.name = UA_STRING((char *)s);
        a.dataType = UA_TYPES[type].typeId;
        a.valueRank = -1; /* scalar */
        push_back(a);
    }
    // TODO add array argument types
};

/*!
    \brief VariantList
*/
typedef std::vector<UA_Variant> VariantList; // shallow copied

// Wrap method call return value sets
/*!
    \brief The VariantCallResult class
*/
// this takes over management of the returned data
class  UA_EXPORT  VariantCallResult {
    UA_Variant *_data = nullptr;
    size_t _size = 0;
public:
    /*!
        \brief VariantCallResult
        \param d
        \param n
    */
    VariantCallResult(UA_Variant *d = nullptr, size_t n = 0) : _data(d), _size(n) {}
    ~VariantCallResult() {
        clear();
    }
    /*!
        \brief clear
    */
    void clear() {
        if (_data) {
            UA_Array_delete(_data, _size, &UA_TYPES[UA_TYPES_VARIANT]);
        }
        _data = nullptr;
        _size = 0;
    }

    /*!
        \brief set
        \param d
        \param n
    */
    void set(UA_Variant *d, size_t n) {
        clear();
        _data = d;
        _size = n;
    }

    /*!
        \brief size
        \return
    */
    size_t size() const {
        return _size;
    }
    /*!
        \brief data
        \return
    */
    UA_Variant *data() const {
        return  _data;
    }

};

/*!
    \brief The VariableAttributes class
*/
class  UA_EXPORT  VariableAttributes : public TypeBase<UA_VariableAttributes> {
public:
    UA_TYPE_DEF(VariableAttributes)
    void setDefault() {
        *this = UA_VariableAttributes_default;
    }

    void setDisplayName(const std::string &s) {
        get().displayName = UA_LOCALIZEDTEXT_ALLOC("en_US", s.c_str());
    }
    void setDescription(const std::string &s) {
        get().description = UA_LOCALIZEDTEXT_ALLOC("en_US", s.c_str());
    }
    void setValue(const Variant &v) {
        UA_Variant_copy(v,  &get().value); // deep copy the variant - do not know life times
    }
    void setValueRank(int i) {
        get().valueRank = i;
    }

    void setHistorizing(bool f = true)
    {
        get().historizing = f;
        if(f)
        {
            get().accessLevel |=  UA_ACCESSLEVELMASK_HISTORYREAD;
        }
        else
        {
            get().accessLevel &= ~UA_ACCESSLEVELMASK_HISTORYREAD;
        }
    }
    void setAccessLevel(UA_Byte b)
    {
        get().accessLevel = b;
    }
};

/*!
    \brief The VariableTypeAttributes class
*/
class  UA_EXPORT  VariableTypeAttributes : public TypeBase<UA_VariableTypeAttributes> {
public:
    UA_TYPE_DEF(VariableTypeAttributes)
    void setDefault() {
        *this = UA_VariableTypeAttributes_default;
    }
    void setDisplayName(const std::string &s) {
        get().displayName = UA_LOCALIZEDTEXT_ALLOC("en_US", s.c_str());
    }
    void setDescription(const std::string &s) {
        get().description = UA_LOCALIZEDTEXT_ALLOC("en_US", s.c_str());
    }
};
/*!
    \brief The MethodAttributes class
*/
class  UA_EXPORT  MethodAttributes : public TypeBase<UA_MethodAttributes> {
public:
    UA_TYPE_DEF(MethodAttributes)
    void setDefault() {
        *this = UA_MethodAttributes_default;
    }

    void setDisplayName(const std::string &s) {
        get().displayName = UA_LOCALIZEDTEXT_ALLOC("en_US", s.c_str());
    }
    void setDescription(const std::string &s) {
        get().description = UA_LOCALIZEDTEXT_ALLOC("en_US", s.c_str());
    }
    //
    /*!
        \brief setExecutable
        \param exe
        \param user
    */
    void setExecutable(bool exe = true, bool user = true) {
        get().executable = exe;
        get().userExecutable = user;
    }

    //
};

/*!
    \brief The Argument class
*/
class  UA_EXPORT  Argument : public TypeBase<UA_Argument> {
public:
    UA_TYPE_DEF(Argument)
    void setDataType(int i) {
        get().dataType = UA_TYPES[i].typeId;
    }
    void setDescription(const std::string &s) {
        get().description = UA_LOCALIZEDTEXT_ALLOC("en_US", s.c_str());
    }
    void setName(const std::string &s) {
        get().name = UA_STRING_ALLOC(s.c_str());
    }
    void setValueRank(int i) {
        get().valueRank = i;   /* scalar argument */
    }
    UA_Argument & set(int type, const std::string &name, const std::string &desc = "", int rank = -1)
    {
        setDataType(type);
        setDescription(name);
        setName(desc);
        setValueRank(rank);
        return get();
    }
};

/*!
    \brief The LocalizedText class
*/
class  UA_EXPORT  LocalizedText : public TypeBase<UA_LocalizedText> {
public:
    UA_TYPE_DEF(LocalizedText)
    LocalizedText(const std::string &locale, const std::string &text) : TypeBase(UA_LocalizedText_new()) {
        get() = UA_LOCALIZEDTEXT_ALLOC(locale.c_str(), text.c_str());
    }
};

/*!
    \brief The RelativePathElement class
*/
class  UA_EXPORT  RelativePathElement : public TypeBase<UA_RelativePathElement> {
public:
    UA_TYPE_DEF(RelativePathElement)
    RelativePathElement(QualifiedName &item, NodeId &typeId, bool inverse = false, bool includeSubTypes = false) :
        TypeBase(UA_RelativePathElement_new()) {
        get().referenceTypeId = typeId.get();
        get().isInverse = includeSubTypes;
        get().includeSubtypes = inverse;
        get().targetName = item.get(); // shallow copy!!!
    }

};


/*!
    \brief The RelativePath class
*/
class  UA_EXPORT  RelativePath : public TypeBase<UA_RelativePath> {
public:
    UA_TYPE_DEF(RelativePath)
};

/*!
    \brief The BrowsePath class
*/

class  UA_EXPORT  BrowsePath : public TypeBase<UA_BrowsePath> {
public:
    UA_TYPE_DEF(BrowsePath)
    /*!
        \brief BrowsePath
        \param start
        \param path
    */
    BrowsePath(NodeId &start, RelativePath &path) : TypeBase(UA_BrowsePath_new()) {
        UA_RelativePath_copy(path.constRef(), &get().relativePath); // deep copy
        UA_NodeId_copy(start, &get().startingNode);
    }
    /*!
        \brief BrowsePath
        \param start
        \param path
    */
    BrowsePath(NodeId &start, RelativePathElement &path) : TypeBase(UA_BrowsePath_new()) {
        get().startingNode = start.get();
        get().relativePath.elementsSize = 1;
        get().relativePath.elements = path.ref();
    }
};

/*!
    \brief The BrowseResult class
*/

class  UA_EXPORT  BrowseResult : public TypeBase<UA_BrowseResult> {
public:
    UA_TYPE_DEF(BrowseResult)
};



class  UA_EXPORT  CallMethodRequest : public TypeBase<UA_CallMethodRequest> {
public:
    UA_TYPE_DEF(CallMethodRequest)
};

class  UA_EXPORT  CallMethodResult  : public TypeBase<UA_CallMethodResult> {
public:
    UA_TYPE_DEF(CallMethodResult)
};

class  UA_EXPORT  ViewAttributes  : public TypeBase<UA_ViewAttributes> {
public:
    UA_TYPE_DEF(ViewAttributes)
    void setDefault() {
        *this = UA_ViewAttributes_default;
    }

};

class  UA_EXPORT  ReferenceTypeAttributes : public TypeBase< UA_ReferenceTypeAttributes> {
public:
    UA_TYPE_DEF(ReferenceTypeAttributes)
    void setDefault() {
        *this = UA_ReferenceTypeAttributes_default;
    }

};

class  UA_EXPORT  DataTypeAttributes : public TypeBase<UA_DataTypeAttributes> {
public:
    UA_TYPE_DEF(DataTypeAttributes)
    void setDefault() {
        *this = UA_DataTypeAttributes_default;
    }

};

class  UA_EXPORT  DataSource : public TypeBase<UA_DataSource> {
public:
    DataSource()  : TypeBase(new UA_DataSource()) {
        get().read = nullptr;
        get().write = nullptr;
    }
};

// Request / Response wrappers for monitored items and events
/*!
    \brief The CreateSubscriptionRequest class
*/
class UA_EXPORT CreateSubscriptionRequest : public TypeBase<UA_CreateSubscriptionRequest> {
public:
    UA_TYPE_DEF(CreateSubscriptionRequest)
};
/*!
    \brief The CreateSubscriptionResponse class
*/
class UA_EXPORT CreateSubscriptionResponse : public TypeBase<UA_CreateSubscriptionResponse> {
public:
    UA_TYPE_DEF(CreateSubscriptionResponse)
};
//
/*!
    \brief The MonitoredItemCreateResult class
*/
class UA_EXPORT MonitoredItemCreateResult : public TypeBase<UA_MonitoredItemCreateResult> {
public:
    UA_TYPE_DEF(MonitoredItemCreateResult)
};


/*!
    \brief The EventFilter class
*/
class UA_EXPORT EventFilter : public TypeBase<UA_EventFilter> {
public:
    UA_TYPE_DEF(EventFilter)
};
/*!
 * \brief The MonitoredItemCreateRequest class
 */
class UA_EXPORT MonitoredItemCreateRequest : public TypeBase<UA_MonitoredItemCreateRequest> {
    //
public:
    UA_TYPE_DEF(MonitoredItemCreateRequest)

    void setItem(const NodeId &nodeId, UA_UInt32 attributeId = UA_ATTRIBUTEID_EVENTNOTIFIER, UA_MonitoringMode monitoringMode = UA_MONITORINGMODE_REPORTING)
    {
        get().itemToMonitor.nodeId = nodeId;
        get().monitoringMode = monitoringMode;
        get().itemToMonitor.attributeId = attributeId;
    }
    void setFilter(UA_EventFilter *filter, UA_ExtensionObjectEncoding encoding = UA_EXTENSIONOBJECT_DECODED, const UA_DataType * type = &UA_TYPES[UA_TYPES_EVENTFILTER]  )
    {
        get().requestedParameters.filter.encoding = encoding;
        get().requestedParameters.filter.content.decoded.data = filter;
        get().requestedParameters.filter.content.decoded.type = type;
    }

    UA_EventFilter *   filter() { return (UA_EventFilter *)(get().requestedParameters.filter.content.decoded.data); }


};

/*!
 * \brief The SetMonitoringModeResponse class
 */
class UA_EXPORT SetMonitoringModeResponse : public TypeBase<UA_SetMonitoringModeResponse> {
public:
    UA_TYPE_DEF(SetMonitoringModeResponse)
};

/*!
 * \brief The SetMonitoringModeRequest class
 */
class UA_EXPORT SetMonitoringModeRequest : public TypeBase<UA_SetMonitoringModeRequest> {
public:
    UA_TYPE_DEF(SetMonitoringModeRequest)
};

/*!
 * \brief The SetTriggeringResult class
 */
class UA_EXPORT SetTriggeringResponse : public TypeBase<UA_SetTriggeringResponse> {
public:
    UA_TYPE_DEF(SetTriggeringResponse)
};

/*!
 * \brief The SetTriggeringRequest class
 */
class UA_EXPORT SetTriggeringRequest : public TypeBase<UA_SetTriggeringRequest> {
public:
    UA_TYPE_DEF(SetTriggeringRequest)
};

//
#if 0
class UA_EXPORT PubSubConnectionConfig : public TypeBase<UA_PubSubConnectionConfig> {
public:
    UA_TYPE_DEF(PubSubConnectionConfig)
};
#endif
//
// Nodes in a browsable / addressable property tree
//
typedef NodePath<std::string> UAPath;
typedef PropertyTree<std::string, NodeId>::PropertyNode UANode;
//
/*!
    \brief The UANodeTree class
*/

class  UA_EXPORT  UANodeTree : public PropertyTree<std::string, NodeId> {

    NodeId _parent; // note parent node
public:
    /*!
        \brief UANodeTree
        \param p
    */
    UANodeTree(NodeId &p): _parent(p) {
        root().setData(p);
    }

    /*!
        \brief ~UANodeTree
    */
    virtual ~UANodeTree() {}
    /*!
        \brief parent
        \return
    */
    NodeId &parent() {
        return  _parent;
    }
    //
    // client and server have different methods - TO DO unify client and server - and template
    // only deal with value nodes and folders - for now
    /*!
        \brief addFolderNode
        \return
    */
    virtual bool addFolderNode(NodeId &/*parent*/, const std::string &/*s*/, NodeId &/*newNode*/) {
        return false;
    }
    /*!
        \brief addValueNode
        \return
    */
    virtual bool addValueNode(NodeId &/*parent*/, const std::string &/*s*/, NodeId &/*newNode*/, Variant &/*v*/) {
        return false;
    }
    /*!
        \brief getValue
        \return
    */
    virtual bool getValue(NodeId &, Variant &) {
        return false;
    }
    /*!
        \brief setValue
        \return
    */
    virtual bool setValue(NodeId &, Variant &) {
        return false;
    }
    //

    /*!
        \brief createPathFolders
        \param p
        \param n
        \param level
        \return
    */
    bool createPathFolders(UAPath &p, UANode *n, int level = 0) {
        bool ret = false;
        if (!n->hasChild(p[level])) {
            NodeId no;
            ret = addFolderNode(n->data(), p[level], no);
            if (ret) {
                auto nn = n->add(p[level]);
                if (nn) nn->setData(no);
            }
        }

        // recurse
        n = n->child(p[level]);
        level++;
        if (level < int(p.size())) {
            ret = createPathFolders(p, n, level);
        }
        //
        return ret;
    }
    /*!
        \brief createPath
        \param p
        \param level
        \return
    */
    bool createPath(UAPath &p, UANode *n, Variant &v, int level = 0) {
        bool ret = false;
        if (!n->hasChild(p[level])) {
            if (level == int(p.size() - 1)) { // terminal node , hence value
                NodeId no;
                ret =  addValueNode(n->data(), p[level], no, v);
                if (ret) {
                    auto nn = n->add(p[level]);
                    if (nn) nn->setData(no);
                }
            }
            else {
                NodeId no;
                ret = addFolderNode(n->data(), p[level], no);
                if (ret) {
                    auto nn = n->add(p[level]);
                    if (nn) nn->setData(no);
                }
            }
        }

        // recurse
        n = n->child(p[level]);
        level++;
        if (level < int(p.size())) {
            ret = createPath(p, n, v, level);
        }
        //
        return ret;
    }

    /*!
        \brief setNodeValue
        \return
    */
    bool setNodeValue(UAPath &p, Variant &v) {
        if (exists(p)) {
            return setValue(node(p)->data(), v); // easy
        }
        else if (p.size() > 0) {
            // create the path and add nodes as needed
            if (createPath(p, rootNode(), v)) {
                return setValue(node(p)->data(), v);
            }
        }
        return false;
    }
    /*!
        \brief getNodeValue
        \param p
        \param v
        \return
    */
    bool getNodeValue(UAPath &p, Variant &v) {
        v.null();
        UANode *np = node(p);
        if (np) { // path exist ?
            return getValue(np->data(), v);
        }
        return false;
    } // get a node if it exists
    /*!
        \brief setNodeValue
        \return
    */
    bool setNodeValue(UAPath &p, const std::string &child, Variant &v) {
        p.push_back(child);
        bool ret = setNodeValue(p, v);
        p.pop_back();
        return ret;
    }
    /*!
        \brief getNodeValue
        \param p
        \param v
        \return
    */
    bool getNodeValue(UAPath &p, const std::string &child, Variant &v) {
        p.push_back(child);
        bool ret = getNodeValue(p, v);
        p.pop_back();
        return ret;
    } // get a node if it exists

    /*!
        \brief printNode
        \param n
        \param os
        \param level
    */
    void printNode(UANode *n, std::ostream &os = std::cerr, int level = 0);

};
//

class UA_EXPORT CreateMonitoredItemsRequest : public TypeBase<UA_CreateMonitoredItemsRequest> {
public:
    UA_TYPE_DEF(CreateMonitoredItemsRequest)
};


// used for select clauses in event filtering
/*!
    \brief SimpleAttributeOperandArray
*/

TYPEDEF_ARRAY(QualifiedName, UA_TYPES_QUALIFIEDNAME)
TYPEDEF_ARRAY(SimpleAttributeOperand,UA_TYPES_SIMPLEATTRIBUTEOPERAND)
/*!
    \brief The EventSelectClause class
*/
class UA_EXPORT EventSelectClauseArray : public SimpleAttributeOperandArray {
public:
    /*!
        \brief EventSelectClause
        \param n
    */
    EventSelectClauseArray(size_t n) : SimpleAttributeOperandArray(n) {
    }


    /*!
     * \brief setClause
     * \param i
     * \param browsePath
     * \param attributeId
     * \param typeDefintion
     * \param indexRange
     */
    void setClause(size_t i, const std::string &browsePath, UA_UInt32 attributeId = UA_ATTRIBUTEID_VALUE,  const NodeId &typeDefintion = NodeId::BaseEventType, const std::string &indexRange = "")
    {
        if(i < length())
        {
            UA_SimpleAttributeOperand &a = data()[i];
            a.typeDefinitionId = typeDefintion;
            a.browsePathSize = 1;
            a.browsePath = (UA_QualifiedName*) UA_Array_new(1, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
            a.attributeId = attributeId;
            a.browsePath[0] = UA_QUALIFIEDNAME_ALLOC(0, browsePath.c_str());
            a.indexRange = toUA_String(indexRange);
        }
    }

    /*!
     * \brief setClause
     * \param i
     * \param browsePath
     * \param attributeId
     * \param typeDefintion
     * \param indexRange
     */
    void setClause(size_t i, StdStringArray &browsePath, UA_UInt32 attributeId = UA_ATTRIBUTEID_VALUE,  const NodeId &typeDefintion = NodeId::BaseEventType, const std::string &indexRange = "")
    {
        if((i < length()) && (browsePath.size() > 0))
        {
            UA_SimpleAttributeOperand &a = data()[i];
            a.typeDefinitionId = typeDefintion;
            a.browsePathSize = browsePath.size();
            a.browsePath = (UA_QualifiedName*) UA_Array_new(a.browsePathSize, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
            a.attributeId = attributeId;
            a.indexRange = toUA_String(indexRange);
            for(size_t j = 0; j < a.browsePathSize; j++)
            {
                a.browsePath[j] = UA_QUALIFIEDNAME_ALLOC(0, browsePath[j].c_str());
            }
        }
    }

    /*!
        \brief setBrowsePath
        \param i
        \param path
    */
    void setBrowsePath(size_t i, UAPath &path) {
        if (i < length()) {
            // allocate array
            QualifiedNameArray bp(path.size());
            // set from the path
            for (size_t j = 0; j < bp.length(); j++) {
                // populate
                const std::string &s = path[j];
                bp.at(j) = UA_QUALIFIEDNAME_ALLOC(0, s.c_str());
            }
            //
            at(i).browsePath =    bp.data();
            at(i).browsePathSize = bp.length();
            bp.release();
            //
        }
    }
    /*!
        \brief setBrowsePath
        \param i
        \param path
    */
    void setBrowsePath(size_t i, const std::string &s) {
        UAPath path;
        path.toList(s);
        setBrowsePath(i, path);
    }

};



/*!
    \brief UAPathArray
    Events work with sets of browse paths
*/
typedef std::vector<UAPath> UAPathArray;




class UA_EXPORT RegisteredServer : public TypeBase<UA_RegisteredServer> {
public:
    UA_TYPE_DEF(RegisteredServer)
};


/*!
    \brief EndpointDescriptionArray
*/
typedef Array<UA_EndpointDescription, UA_TYPES_ENDPOINTDESCRIPTION> EndpointDescriptionArray;
/*!
    \brief ApplicationDescriptionArray
*/
typedef Array<UA_ApplicationDescription, UA_TYPES_APPLICATIONDESCRIPTION> ApplicationDescriptionArray;
/*!
    \brief ServerOnNetworkArray
*/
typedef Array<UA_ServerOnNetwork, UA_TYPES_SERVERONNETWORK> ServerOnNetworkArray;
//
// Forward references
//
class UA_EXPORT ClientSubscription;
class UA_EXPORT MonitoredItem;
class UA_EXPORT Server;
class UA_EXPORT Client;
class UA_EXPORT SeverRepeatedCallback;
//
typedef std::list<BrowseItem> BrowseList;
/*!
    \brief The BrowserBase class
    NOde browsing base class
*/
class UA_EXPORT BrowserBase {
protected:
    BrowseList _list;
    static UA_StatusCode browseIter(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle);
public:
    BrowserBase() = default;
    virtual ~BrowserBase() {
        _list.clear();
    }
    BrowseList &list() {
        return _list;
    }
    virtual void browse(UA_NodeId /*start*/) {}
    virtual bool browseName(NodeId &/*n*/, std::string &/*s*/, int &/*i*/) {
        return false;
    }

    /*!
        \brief print
        \param os
    */
    void print(std::ostream &os);
    /*!
        \brief find
        \param s
        \return
    */
    BrowseList::iterator find(const std::string &s);
    /*!
        \brief process
        \param childId
        \param referenceTypeId
    */
    void process(UA_NodeId childId,  UA_NodeId referenceTypeId);
};

template <typename T>
/*!
    \brief The Browser class
*/
class Browser : public BrowserBase {
    T &_obj;
    // browser call back
    BrowseList _list;
    //
public:
    Browser(T &c) : _obj(c) {}
    T &obj() {
        return _obj;
    }

    bool browseName(NodeId &n, std::string &s, int &i) {
        return _obj.browseName(n, s, i);
    }

};


// debug helpers
std::string  timestampToString(UA_DateTime date);
std::string  dataValueToString(UA_DataValue *value);
std::string variantToString(UA_Variant &v);
}
#endif // OPEN62541OBJECTS_H
