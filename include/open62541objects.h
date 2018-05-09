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
#include "open62541.h"
#include "trace.h"
//
#include <string>
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
    // reference counted shared pointer wrappers for Open types
    //
    // browse path
    //
    template<typename T> class UA_EXPORT TypeBase {
        protected:
            std::unique_ptr<T> _d; // shared pointer - there is no copy on change
        public:
            TypeBase(T *t) : _d(t) {}
            //TypeBase(const TypeBase &t) : _d(t._d) {}
            T &get() {
                return *(_d.get());
            }
            // Reference and pointer for parameter passing
            operator T &() {
                return get();
            }

            operator T *() {
                return _d.get();
            }
            const T *constRef() {
                return _d.get();
            }

            T   *ref() {
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

#define UA_TYPE_BASE(C,T)\
    C() :  TypeBase(T##_new()) {T##_init(_d.get()); UA_TRC("Construct:" << UA_STRINGIFY(C))}\
    C(const T &t) :  TypeBase(T##_new()){assignFrom(t);UA_TRC("Construct (" << UA_STRINGIFY(T) << ")")}\
    ~C(){UA_TRC("Delete:" << UA_STRINGIFY(C)); if(_d) T##_deleteMembers(_d.get());}\
    C(const C & n) :  TypeBase(T##_new())  { T##_copy(n._d.get(),_d.get()); UA_TRC("Copy Construct:" << UA_STRINGIFY(C))}\
    C & operator = ( const C &n) {UA_TRC("Assign:" << UA_STRINGIFY(C));null(); T##_copy(n._d.get(),_d.get()); return *this;}\
    void null() {if(_d){UA_TRC("Delete(in null):" << UA_STRINGIFY(C));T##_deleteMembers(_d.get());} _d.reset(T##_new());T##_init(_d.get());}\
    void assignTo(T &v){ T##_copy(_d.get(),&v);}\
    void assignFrom(const T &v){ T##_copy(&v,_d.get());}


#define UA_TYPE_DEF(T) UA_TYPE_BASE(T,UA_##T)


    template <typename T, const int I>
    /*!
           \brief The Array class
           This is for allocated arrays of UA_ objects
           simple lifecycle management.
           Uses UA_array_new and UA_array_delete
           rather than new and delete
           Also deals with array sreturned from UA_ functions
    */
    class Array {
            size_t _length = 0;
            T *_data = nullptr;
        public:
            Array() {}
            Array(T *data, size_t len)
                : _data(data), _length(len) {
            }

            Array(size_t n) {
                allocate(n);
            }

            ~Array() {
                clear();
            }

            /*!
                \brief allocate
                \param len
            */
            void allocate(size_t len) {
                clear();
                _data = (T *)(UA_Array_new(len, &UA_TYPES[I]));
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

            /*!
                \brief clear
            */
            void clear() {
                if (_length && _data) {
                    UA_Array_delete(_data, _length, &UA_TYPES[I]);
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
    };

    //
    // typedef basic array types
    typedef Array<UA_String, UA_TYPES_STRING> StringArray;
    typedef Array<UA_NodeId, UA_TYPES_NODEID> NodeIdArray;

    // none heap allocation - no delete
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
        UA_String_deleteMembers(&r);
        r = UA_STRING_ALLOC(s.c_str());
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
                UA_String_deleteMembers(&ref()->username);
                UA_String_deleteMembers(&ref()->password);
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
    class UA_EXPORT NodeClass : public TypeBase<UA_NodeClass> {
        public:
            UA_TYPE_DEF(NodeClass)
    };
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

            //
            UA_TYPE_DEF(NodeId)
            //
            // null
            //
            bool isNull() {
                return UA_NodeId_isNull(ref());
            }

            // equality
            bool operator == (const NodeId &n) {
                return UA_NodeId_equal(_d.get(), n._d.get());
            }
            /* Returns a non-cryptographic hash for the NodeId */
            unsigned hash() {
                return UA_NodeId_hash(ref());
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
            int nameSpaceIndex() {
                return ref()->namespaceIndex;
            }
            UA_NodeIdType identifierType() {
                return ref()->identifierType;
            }
            //
            NodeId &notNull() { // makes a node not null so new nodes are returned to references
                null(); // clear anything beforehand
                *(_d.get()) = UA_NODEID_NUMERIC(1, 0); // force a node not to be null
                return *this;
            }
    };

    /*!
        \brief toString
        \param n
        \return
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
                    UA_NodeId_deleteMembers(&(at(i))); // delete members
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
                    UA_NodeId_deleteMembers(&n);
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
    };



    /*!
        \brief toString
        \param r
        \return
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
    // Memory Leak Risk - TODO Check this
    //

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

            Variant(UA_UInt64 v) : TypeBase(UA_Variant_new()) {
                UA_Variant_setScalarCopy((UA_Variant *)ref(), &v, &UA_TYPES[UA_TYPES_UINT64]);
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
                        UA_Variant_deleteMembers((UA_Variant *)ref());
                    }
                }
            }

            // convert from an any to Variant
            // limit to basic types
            void fromAny(boost::any &a);


    };

    typedef Array<UA_Variant, UA_TYPES_VARIANT> VariantArray;

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
            void setValue(Variant &v) {
                UA_Variant_copy(v,  &get().value); // deep copy the variant - do not know life times
            }
            void setValueRank(int i) {
                get().valueRank = i;
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

    /*!
        \brief The BrowsePathResult class
    */
    class  UA_EXPORT  BrowsePathResult : public TypeBase<UA_BrowsePathResult> {
        public:
            UA_TYPE_DEF(BrowsePathResult)
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

    class UA_EXPORT MonitoredItemCreateRequest : public TypeBase<UA_MonitoredItemCreateRequest> {
        public:
            UA_TYPE_DEF(MonitoredItemCreateRequest)
    };

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
             * \brief ~UANodeTree
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
    typedef Array<UA_SimpleAttributeOperand, UA_TYPES_SIMPLEATTRIBUTEOPERAND> SimpleAttributeOperandArray;
    typedef Array<UA_QualifiedName, UA_TYPES_QUALIFIEDNAME> QualifiedNameArray;
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
                for (size_t i = 0; i < n; i++) {
                    at(i).attributeId =  UA_ATTRIBUTEID_VALUE;
                    at(i).typeDefinitionId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEEVENTTYPE);
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
    /*!
        \brief The EventFilter class
    */
    class UA_EXPORT EventFilter : public TypeBase<UA_EventFilter> {
        public:
            UA_TYPE_DEF(EventFilter)
    };

    /*!
        \brief The EventFilterSelect class
    */
    class UA_EXPORT EventFilterSelect : public EventFilter {
            EventSelectClauseArray _selectClause; // these must have the life time of the monitored event
        public:
            EventFilterSelect() = default;
            /*!
                \brief EventFilter
                \param i
            */
            EventFilterSelect(size_t i) : _selectClause(i) {

            }
            /*!
                \brief selectClause
                \return
            */
            EventSelectClauseArray &selectClause() {
                return _selectClause;
            }

            /*!
             */
            ~EventFilterSelect()
            {
                _selectClause.clear();
            }


            /*!
                \brief setBrowsePaths
                \param a
            */
            void setBrowsePaths(UAPathArray &a) {
                //UAPath has all the vector stuff and can parse string paths
                if (a.size()) {
                    if (a.size() == _selectClause.length()) {
                        for (size_t i = 0; i < a.size(); i++) {
                            _selectClause.setBrowsePath(i, a[i]); // setup a set of browse paths
                        }
                    }
                }
            }

    };

    class UA_EXPORT RegisteredServer : public TypeBase<UA_RegisteredServer> {
        public:
            UA_TYPE_DEF(RegisteredServer)
    };

    typedef std::unique_ptr<EventFilterSelect> EventFilterRef;

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
}
#endif // OPEN62541OBJECTS_H
