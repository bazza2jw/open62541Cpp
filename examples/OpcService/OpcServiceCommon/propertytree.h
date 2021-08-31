#ifndef PROPERTYTREE_H
#define PROPERTYTREE_H

/*!

    \author B. J. Hill
    \date __DATE__
    License:  GNU LESSER GENERAL PUBLIC LICENSE 2.1
    (c)  Micro Research Limited 2010 -
*/
#include "mrlmutex.h"
#include <string>
#include <vector>
#include <map>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <iterator>
#include <algorithm>
#include <ostream>
#include <functional>
#include <list>

namespace MRL {

// a tree is an addressable set of nodes
// objects of type T must have an assignment operator
//
typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
/*!
       \brief The NodePath class

*/
template <typename T>
class NodePath : public std::list<T>
{
public:
    /*!
           \brief NodePath
    */
    NodePath() {}
    /*!
        \brief NodePath
        \param n
    */
    NodePath(const NodePath<T>& n)
        : std::list<T>(n)
    {
    }
    /*!
        \brief toList
        Converts a string to a list
        \param s string to split
        \param seperator the separator between fields
    */
    void toList(const T& s, const char* seperator = ".")
    {
        boost::char_separator<char> sep(seperator);
        tokenizer tokens(s, sep);
        for (auto i = tokens.begin(); i != tokens.end(); i++) {
            this->push_back(*i);
        }
    }
    /*!
        \brief toString
        \param s receives concatenated path elements
    */
    void toString(T& s, const char* seperator = ".") const
    {
        if (this->size() > 0) {
            const NodePath& n = *this;
            auto i            = n.begin();
            s                 = *i;
            i++;
            for (; i != n.end(); i++) {
                s += seperator;
                s += *i;
            }
        }
    }

    /*!
        \brief append
        Append a path to this path
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
/*!
    \brief The Node class
    This is a leaf in the property tree
*/
class Node
{
public:
    typedef std::map<K, Node*> ChildMap;  //!< map of children
    typedef NodePath<K> Path;             //!< path in a tree
private:
    K _name;                  //!< the name of the node
    T _data;                  //!< the leaf data
    Node* _parent = nullptr;  //!< the node's parent
    ChildMap _children;       //!< the children
public:
    /*!
     * \brief The NodeIteratorFunc class
     * A non-lambda visitor class
     */
    class NodeIteratorFunc
    {
    public:
        virtual void Do(Node*) {}
    };

    /*!
        \brief Node
        Construct
    */
    Node() {}
    /*!
        \brief Node
        Construct
        \param name browse name
        \param parent parent node, nullptr if none
    */
    Node(const K& name, Node* parent = nullptr)
        : _name(name)
        , _parent(parent)
    {
    }

    /*!
        \brief ~Node
        Destruct and deregister from parent
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
        Delete all chaildren
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
        \return dictionary of children
    */
    ChildMap& children() { return _children; }

    /*!
     * \brief hasChildren
     * \return true if the node has children
     */
    bool hasChildren() { return _children.size() > 0; }

    /*!
        \brief data
        \return reference to data
    */
    T& data() { return _data; }

    /*!
        \brief setData
        Set the data value. The data type must have a copy constructor
        \param d
    */
    void setData(const T& d) { _data = d; }

    /*!
        \brief child
        \param s browse name of child to find
        \return pointer to child or nullptr
    */
    Node* child(const K& s) { return _children[s]; }

    /*!
        \brief hasChild
        \param s browse name of child
        \return true if child exists
    */
    bool hasChild(const K& s) { return _children[s] != nullptr; }
    /*!
        \brief addChild
        \param key browse name of child
        \param n pointer to child object
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
        Creates a child with default data
        \param s browse name
        \param p parent node, this node by default
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
        \param s browse name
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
        \return name of node
    */
    const K& name() const { return _name; }
    /*!
        \brief setName
        \param s name of node
    */
    void setName(const K& s) { _name = s; }
    /*!
        \brief parent
        \return parent Node pointer
    */
    Node* parent() const { return _parent; }
    /*!
        \brief setParent
        Set / replace parent
        \param p parent node
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
    Node* find(const Path& path)
    {
        Node* res = this;
        for (auto depth = path.begin(); (depth != path.end()) && (res != nullptr); depth++) {
            res = res->child(*depth);
        }
        return res;
    }

    /*!
        \brief find
        find a node given the path
        \param path path to search
        \return node pointer of nullptr if not present
    */
    Node* find(const K& path)
    {
        Path p;
        p.toList(path);
        return find(p);
    }

    /*!
        \brief add
        Adds a node to the tree starting with this node
        \param p path with respect to this node
        \return new node or nullptr
    */
    Node* add(const Path& p)
    {
        Node* n = find(p);  // only create if it does not exist
        if (!n) {
            // create the path as required
            n          = this;
            auto depth = p.begin();
            while (n->hasChild(*depth)) {
                n = n->child(*depth);
                depth++;
            }
            // create the rest
            for (; depth != p.end(); depth++) {
                n = n->createChild(*depth);
            }
        }
        //
        return n;  // return the newly created node
    }

    /*!
        \brief add
        Add a node by string path
        \param path as string type
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
        remove a node by Path
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
        Remove a node by string path
        \param s
    */
    void remove(const K& s)
    {
        Path p;
        p.toList(s);
        remove(p);
    }

    /*!
        \brief iterateNodes
        Iterate this node and all children using the given lambda
        \param func lambda to use to visit items with
        \return
    */
    bool iterateNodes(std::function<bool(Node&)> func)
    {
        if (func(*this)) {
            if (hasChildren()) {
                for (auto i = children().begin(); i != children().end(); i++) {
                    (i->second)->iterateNodes(func);
                }
            }
            return true;
        }
        return false;
    }

    /*!
        \brief iterateNodes
        Non-lambda visitor form
        \param n
    */
    void iterateNodes(NodeIteratorFunc& n)
    {
        n.Do(this);  // action the function for the node
        if (hasChildren()) {
            for (auto i = children().begin(); i != children().end(); i++) {
                (i->second)->iterateNodes(n);
            }
        }
    }
    /*!
        \brief write
        write the node and its children to the stream
        \param os output stream
    */
    template <typename STREAM>
    void write(STREAM& os)
    {
        os << name();
        os << data();
        os << static_cast<int>(children().size());  // this node's data
        // now recurse
        if (children().size() > 0) {
            if (hasChildren()) {
                for (auto i = children().begin(); i != children().end(); i++) {
                    i->second->write(os);
                }
            }
        }
    }

    /*!
        \brief read
        Read a node and its children from a stream
        \param is the stream
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
        \param n Node to copy to
    */
    void copyTo(Node* n)
    {
        n->clear();
        n->setName(name());
        n->setData(data());
        if (children().size() > 0) {
            for (auto i = children().begin(); i != children().end(); i++) {
                Node* c = new Node();
                c->addChild(n);        // add the child
                i->second->copyTo(c);  // now recurse
            }
        }
    }
};

template <typename K, typename T>
/*!
 * \brief The PropertyTree class
 */
class PropertyTree
{
    mutable ReadWriteMutex _mutex;  //!< mutex for read/write access
    bool _changed = false;          //!< track if any action may have changed the tree
public:
    //
    T _defaultData;  //!< default data to be returned
    typedef Node<K, T> PropertyNode;
    typedef NodePath<K> Path;
    //
private:
    PropertyNode _empty;  //!< empty node
    PropertyNode _root;   //!< the root node
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
        \return mutex
    */
    ReadWriteMutex& mutex() { return _mutex; }
    /*!
        \brief changed
        \return changed flag
    */
    bool changed() const { return _changed; }
    /*!
        \brief clearChanged
        Clears the changed flag
    */
    void clearChanged() { _changed = false; }
    /*!
        \brief setChanged
        \param f value to set the changed flag to
    */
    void setChanged(bool f = true) { _changed = f; }
    /*!
        \brief clear
        clear / delete all nodes (other than root) from the tree
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
    /*!
     * \brief get
     * Get a data value reference from the tree by path
     * \param path the path
     * \return reference to object or default value if not found
     */
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
        \return reference to root node
    */
    PropertyNode& root() { return _root; }

    /*!
        \brief rootNode
        \return pointer to root node
    */
    PropertyNode* rootNode() { return &this->_root; }

    template <typename P>
    /*!
     * \brief node
     * Find a node by path
     * \param path
     * \return pointer to node or nullptr
     */
    PropertyNode* node(const P& path)
    {
        ReadLock l(_mutex);
        return _root.find(path);
    }

    template <typename P>
    /*!
     * \brief set
     * Set data for a node. Path is created if necessary
     * \param path path to item
     * \param d data
     */
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

    template <typename P>
    /*!
     * \brief exists
     * \param path
     * \return true if the item exists in the tree
     */
    bool exists(const P& path)
    {
        return _root.find(path) != nullptr;
    }

    template <typename P>
    /*!
     * \brief remove
     * remove a node from the tree
     * \param path
     */
    void remove(const P& path)
    {
        WriteLock l(_mutex);
        setChanged();
        _root.remove(path);
    }

    /*!
        \brief absolutePath
        Get the full path to a node
        \param n node to find path for
        \param p receives the path
    */
    void absolutePath(PropertyNode* n, Path& p)
    {
        p.clear();
        if (n) {
            ReadLock l(_mutex);
            while (n->parent() != nullptr) {
                p.push_front(n->name());
                n = n->parent();
            }
        }
    }

    /*!
        \brief getChild
        Get child of a node
        \param node
        \param s
        \param def
        \return reference to the data or default
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
        Set the data for a child
        \param node
        \param s child name
        \param v data
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
        Iterates all nodes with func
        \param func lamda to visit nodes with
        \return true if iteration is to continue
    */
    bool iterateNodes(std::function<bool(PropertyNode&)> func)
    {
        ReadLock l(_mutex);
        return _root.iterateNodes(func);
    }

    /*!
        \brief write
        Write on a stream
    */
    template <typename S>
    void write(S& os)
    {
        ReadLock l(_mutex);
        _root.write(os);
    }

    /*!
        \brief read
        Read from a stream
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
        Copy to a tree
        \param dest destination tree
    */
    void copyTo(PropertyTree& dest)
    {
        ReadLock l(_mutex);
        _root.copyTo(&dest._root);
        dest.setChanged();
    }

    template <typename P>
    /*!
        \brief listChildren
        List the children of a node
        \param path  path to node
        \param l receives the list of child node names
    */
    int listChildren(const P& path, std::list<K>& l)
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

    /*!
     * \brief iterateWithPath
     * Iterate a node and its children
     * \param in
     * \param p
     * \param func
     */
    void iterateWithPath(PropertyNode* in, Path& p, std::function<void(PropertyNode* in, Path& p)> func)
    {
        func(in, p);
        for (auto i = in->children().begin(); i != in->children().end(); i++) {
            if (i->second) {
                p.push_back(i->first);
                iterateWithPath(i->second, p, func);
                p.pop_back();
            }
        }
    }

    /*!
     * \brief iterateWithPath
     * Iterate all nodes passing in path as well
     * \param func
     */
    void iterateWithPath(std::function<void(PropertyNode* in, Path& p)> func)
    {
        Path p;
        iterateWithPath(rootNode(), p, func);
    }
};
}  // namespace MRL
#endif  // PROPERTYTREE_H
