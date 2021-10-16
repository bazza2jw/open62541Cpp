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
#ifndef UA_PROPERTYTREE_H
#define UA_PROPERTYTREE_H
#include <boost/optional/optional.hpp>
#include <boost/thread.hpp>
#include <boost/thread/locks.hpp>
#include <boost/bind/bind.hpp>
#include <string>
#include <vector>
#include <map>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <iterator>
#include <algorithm>
#include <ostream>
#include <functional>

// Mutexs
//
namespace Open62541 {

typedef boost::shared_mutex ReadWriteMutex;
typedef boost::shared_lock<boost::shared_mutex> ReadLock;
typedef boost::unique_lock<boost::shared_mutex> WriteLock;

// a tree is an addressable set of nodes
// objects of type T must have an assignment operator
//
typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
/*!
       \brief The NodePath class
*/
template <typename T>
class NodePath : public std::vector<T>
{
public:
    NodePath() {}
    /*!
        \brief toList
        \param s
        \param seperator
    */
    void toList(const T& s, const char* seperator = "/")
    {
        boost::char_separator<char> sep(seperator);
        tokenizer tokens(s, sep);
        for (auto i = tokens.begin(); i != tokens.end(); i++) {
            this->push_back(*i);
        }
    }
    /*!
        \brief toString
        \param s
    */
    void toString(T& s, const char* seperator = "/")
    {
        if (this->size() > 0) {
            NodePath& n = *this;
            s           = n[0];
            for (unsigned i = 1; i < this->size(); i++) {
                s += seperator;
                s += n[i];
            }
        }
    }

    /*!
        \brief append
        \param p
    */
    const NodePath<T>& append(const NodePath<T>& p)
    {
        // append a child path
        for (int i = 0; i < int(p.size()); i++) {
            this->push_back(p[i]);
        }
        return *this;
    }
};

template <typename K, typename T>
class Node
{
public:
    typedef std::map<K, Node*> ChildMap;
    typedef NodePath<K> Path;

private:
    // the name of the node
    K _name;
    // the node data
    T _data;
    // the node's parent
    Node* _parent = nullptr;
    //
    ChildMap _children;
    //
public:
    class NodeIteratorFunc
    {
    public:
        virtual void Do(Node*) {}
    };

    /*!
        \brief Node
    */
    Node() {}
    /*!
        \brief Node
        \param name
        \param parent
    */
    Node(const K& name, Node* parent = nullptr)
        : _name(name)
        , _parent(parent)
    {
    }

    /*!
        \brief ~Node
    */
    virtual ~Node()
    {
        if (_parent) {
            _parent->_children.erase(name());  // detach
            _parent = nullptr;
        }
        clear();
    }

    /*!
        \brief clear
    */
    void clear()
    {
        for (auto i = _children.begin(); i != _children.end(); i++) {
            Node* n = i->second;
            if (n) {
                n->_parent = nullptr;
                delete n;
            }
        }
        _children.clear();
    }

    /*!
        \brief children
        \return
    */
    ChildMap& children() { return _children; }

    /*!
        \brief data
        \return
    */
    T& data() { return _data; }

    /*!
        \brief setData
        \param d
    */
    void setData(const T& d) { _data = d; }

    /*!
        \brief child
        \param s
        \return
    */
    Node* child(const K& s) { return _children[s]; }

    /*!
        \brief hasChild
        \param s
        \return
    */
    bool hasChild(const K& s) { return _children[s] != nullptr; }
    /*!
        \brief addChild
        \param key
        \param n
    */
    void addChild(Node* n)
    {
        if (hasChild(n->name())) {
            delete _children[n->name()];
            _children.erase(n->name());
        }
        _children[n->name()] = n;
    }

    /*!
        \brief createChild
        \param s
        \param p
        \return
    */
    Node* createChild(const K& s, Node* p = nullptr)
    {
        if (!p)
            p = this;
        Node* n = new Node(s, p);
        addChild(n);
        return n;
    }

    /*!
        \brief removeChild
        \param s
    */
    void removeChild(const K& s)
    {
        if (hasChild(s)) {
            Node* n = child(s);  // take the child node
            _children.erase(s);
            if (n)
                delete n;
        }
    }
    // accessors
    /*!
        \brief name
        \return
    */
    const K& name() const { return _name; }
    /*!
        \brief setName
        \param s
    */
    void setName(const K& s) { _name = s; }
    /*!
        \brief parent
        \return
    */
    Node* parent() const { return _parent; }
    /*!
        \brief setParent
        \param p
    */
    void setParent(Node* p)
    {
        if (_parent && (_parent != p)) {
            _parent->_children.erase(name());
        }

        _parent = p;
        if (_parent)
            _parent->_children[name()] = this;
    }

    /*!
        \brief find
        \param path
        \param depth
        \return nullptr on failure
    */
    Node* find(const Path& path, int depth = 0)
    {
        Node* res = child(path[depth]);  // do we have the child at this level?
        if (res) {
            depth++;
            if (depth < (int)path.size()) {
                res = res->find(path, depth);
            }
        }
        return res;
    }

    /*!
        \brief find
        \param path
        \return
    */
    Node* find(const K& path)
    {
        Path p;
        p.toList(path);
        return find(p);
    }

    /*!
        \brief add
        \param p
        \return
    */
    Node* add(const Path& p)
    {
        Node* n = find(p);  // only create if it does not exist
        if (!n) {
            // create the path as required
            n         = this;
            int depth = 0;
            while (n->hasChild(p[depth])) {
                n = n->child(p[depth]);
                depth++;
            }
            // create the rest
            for (unsigned i = depth; i < p.size(); i++) {
                n = n->createChild(p[i]);
            }
        }
        //
        return n;  // return the newly created node
    }

    /*!
        \brief add
        \param path
        \return
    */
    Node* add(const K& path)
    {
        Path p;
        p.toList(path);
        return add(p);
    }

    /*!
        \brief remove
        \param path
    */
    void remove(const Path& path)
    {
        Node* p = find(path);

        if (p) {
            delete p;
        }
    }

    /*!
        \brief remove
        \param s
    */
    void remove(const K& s)
    {
        Path p;
        p.toList(s);
        remove(p);
    }

    /*!
        \brief iterateNodes - iterate this node and all children using the given lambda
        \param func
        \return
    */
    bool iterateNodes(std::function<bool(Node&)> func)
    {
        if (func(*this)) {
            for (auto i = children().begin(); i != children().end(); i++) {
                (i->second)->iterateNodes(func);
            }
            return true;
        }
        return false;
    }

    /*!
        \brief iterateNodes
        \param n
    */
    void iterateNodes(NodeIteratorFunc& n)
    {
        n.Do(this);  // action the function for the node
        for (auto i = children().begin(); i != children().end(); i++) {
            i->second->iterateNodes(n);
        }
    }
    /*!
        \brief write
        \param os
    */
    template <typename STREAM>
    void write(STREAM& os)
    {
        os << name();
        os << data();
        os << static_cast<int>(children().size());  // this node's data
        // now recurse
        if (children().size() > 0) {
            for (auto i = children().begin(); i != children().end(); i++) {
                i->second->write(os);
            }
        }
    }

    /*!
        \brief read
        \param is
    */
    template <typename STREAM>
    void read(STREAM& is)
    {
        int n = 0;
        clear();
        is >> _name;
        is >> _data;
        is >> n;
        if (n > 0) {
            for (int i = 0; i < n; i++) {
                Node* o = new Node();
                o->read(is);  // recurse
                addChild(o);  // add subtree to children
            }
        }
    }

    /*!
        \brief copyTo
        recursive copy
        \param n
    */
    void copyTo(Node* n)
    {
        n->clear();
        n->setName(name());
        n->setData(data());
        if (children().size() > 0) {
            for (auto i = children().begin(); i != children().end(); i++) {
                Node* c = new Node();
                n->addChild(c);        // add the child
                i->second->copyTo(c);  // now recurse
            }
        }
    }
};
/*!

*/
template <typename K, typename T>
class PropertyTree
{
    mutable ReadWriteMutex _mutex;
    bool _changed = false;

public:
    T _defaultData;
    typedef Node<K, T> PropertyNode;
    typedef NodePath<K> Path;

private:
    PropertyNode _empty;  // empty node
    PropertyNode _root;   // the root node
public:
    /*!
        \brief PropertyTree
    */
    PropertyTree()
        : _empty("__EMPTY__")
        , _root("__ROOT__")
    {
        _root.clear();
    }
    /*!
        \brief ~PropertyTree
    */
    virtual ~PropertyTree() { _root.clear(); }

    /*!
        \brief mutex
        \return
    */
    ReadWriteMutex& mutex() { return _mutex; }
    /*!
        \brief changed
        \return
    */
    bool changed() const { return _changed; }
    /*!
        \brief clearChanged
    */
    void clearChanged() { _changed = false; }
    /*!
        \brief setChanged
        \param f
    */
    void setChanged(bool f = true) { _changed = f; }
    /*!
        \brief clear
    */
    void clear()
    {
        WriteLock l(_mutex);
        _root.clear();
        setChanged();
    }

    /*!

    */
    template <typename P>
    T& get(const P& path)
    {
        ReadLock l(_mutex);
        auto* p = _root.find(path);
        if (p) {
            return p->data();
        }
        return _defaultData;
    }

    /*!
        \brief root
        \return
    */
    PropertyNode& root() { return _root; }

    /*!
        \brief rootNode
        \return
    */
    PropertyNode* rootNode() { return &this->_root; }

    /*!
        \brief node
    */
    template <typename P>
    PropertyNode* node(const P& path)
    {
        ReadLock l(_mutex);
        return _root.find(path);
    }

    /*!
        \brief set
    */
    template <typename P>
    PropertyNode* set(const P& path, const T& d)
    {
        auto p = _root.find(path);
        if (!p) {
            WriteLock l(_mutex);
            p = _root.add(path);
        }
        if (p) {
            WriteLock l(_mutex);
            p->setData(d);
        }
        setChanged();
        return p;
    }

    /*!
        \brief exists
    */
    template <typename P>
    bool exists(const P& path)
    {
        return _root.find(path) != nullptr;
    }

    /*!

    */
    template <typename P>
    void remove(const P& path)
    {
        WriteLock l(_mutex);
        setChanged();
        _root.remove(path);
    }

    /*!
        \brief absolutePath
        \param n
        \param p
    */
    void absolutePath(PropertyNode* n, Path& p)
    {
        p.clear();
        if (n) {
            ReadLock l(_mutex);
            do {
                p.push_back(n->name());
                n = n->parent();
            } while (n != nullptr);
            std::reverse(std::begin(p), std::end(p));
        }
    }

    /*!
        \brief getChild
        \param node
        \param s
        \param def
        \return
    */
    T& getChild(PropertyNode* node, const K& s, T& def)
    {
        ReadLock l(_mutex);
        if (node && node->hasChild(s)) {
            return node->child(s)->data();
        }
        return def;
    }

    /*!
        \brief setChild
        \param node
        \param s
        \param v
    */
    void setChild(PropertyNode* node, const K& s, const T& v)
    {
        if (node) {
            WriteLock l(_mutex);
            if (node->hasChild(s)) {

                node->child(s)->setData(v);
            }
            else {
                PropertyNode* c = node->createChild(s);
                c->setData(v);
            }
            setChanged();
        }
    }

    /*!
        \brief iterateNodes
        \param func
        \return
    */
    bool iterateNodes(std::function<bool(PropertyNode&)> func)
    {
        WriteLock l(_mutex);
        return _root.iterateNodes(func);
    }

    /*!
        \brief write
    */
    template <typename S>
    void write(S& os)
    {
        ReadLock l(_mutex);
        _root.write(os);
    }

    /*!
        \brief read
    */
    template <typename S>
    void read(S& is)
    {
        WriteLock l(_mutex);
        _root.read(is);
        setChanged();
    }

    /*!
        \brief copyTo
        \param dest
    */
    void copyTo(PropertyTree& dest)
    {
        ReadLock l(_mutex);
        _root.copyTo(&dest._root);
        dest.setChanged();
    }

    template <typename P>
    /*!
        \brief list
        \param path
        \param l
    */
    int listChildren(const P& path, std::vector<K>& l)
    {
        auto i = node(path);
        if (i) {
            ReadLock lx(_mutex);
            for (auto j = i->children().begin(); j != i->children().end(); j++) {
                l.push_back(j->first);
            }
        }
        return l.size();
    }
};
}  // namespace Open62541
#endif  // PROPERTYTREE_H
