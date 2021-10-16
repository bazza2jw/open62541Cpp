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
#ifndef NODECONTEXT_H
#define NODECONTEXT_H
#include <open62541cpp/open62541objects.h>
namespace Open62541 {
/*!
    \brief The NodeContext class
    Node context objects operate on nodes - the node contexts may be shared between more than one node
    As of version 0.3 the node context is passed to callbacks and so can be used
    This aggregates the data value call backs and value call backs and lifecycle callbacks
*/
class UA_EXPORT NodeContext
{
    std::string _name;                               // Context name
    static UA_DataSource _dataSource;                //!< Call back for data source operations
    static UA_ValueCallback _valueCallback;          //!< call back for value get / set
    static UA_NodeTypeLifecycle _nodeTypeLifeCycle;  //!< life cycle callback
public:
    typedef std::function<bool(Server&, NodeId&, const UA_NumericRange*, UA_DataValue&)> DataFunc;
    typedef std::function<void(Server&, NodeId&, const UA_NumericRange*, const UA_DataValue*)> ValueFunc;
    typedef std::function<bool(Server&, NodeId&, const UA_NumericRange*, const UA_DataValue&)> ConstDataFunc;
    typedef std::function<void(Server&, NodeId&, const UA_NumericRange*, const UA_DataValue&)> ConstValueFunc;

protected:
    UA_StatusCode _lastError;
    // Functor read write interface
    DataFunc _readData;
    ConstDataFunc _writeData;
    ValueFunc _readValue;
    ConstValueFunc _writeValue;

public:
    //
    /*!
        \brief NodeContext
        \param s
    */
    NodeContext(const std::string& s = "")
        : _name(s)
    {
    }

    /*!
     * \brief NodeContext
     * \param s
     * \param read
     * \param write
     */
    NodeContext(DataFunc read, ConstDataFunc write, const std::string& s = "")
        : _name(s)
        , _readData(read)
        , _writeData(write)
    {
        // Data read write node
    }

    /*!
     * \brief NodeContext
     * \param s
     * \param read
     * \param write
     */

    NodeContext(ValueFunc read, ConstValueFunc write, const std::string& s = "")
        : _name(s)
        , _readValue(read)
        , _writeValue(write)
    {
        // Value read/write node
    }

    /*!
     * \brief ~NodeContext
     */
    virtual ~NodeContext() {}

    // accessors
    void setReadData(DataFunc f) { _readData = f; }
    void setWriteData(ConstDataFunc f) { _writeData = f; }
    void setReadValue(ValueFunc f) { _readValue = f; }
    void setWriteValue(ConstValueFunc f) { _writeValue = f; }

    /*!
        \brief lastError
        \return
    */
    UA_StatusCode lastError() const { return _lastError; }

    /*!
        \brief lastOK
        \return last error code
    */
    bool lastOK() const { return _lastError == UA_STATUSCODE_GOOD; }

    /*!
     * \brief name
     * \return
     */
    const std::string& name() { return _name; }
    /*!
        \brief find
        \param s
        \return
    */

    // global life-cycle construct and destruct
    /*!
        \brief construct
        \param node
    */
    virtual bool construct(Server&, NodeId&)
    {
        return true;  // doing nothing is OK
    }

    /*!
        \brief destruct
    */
    virtual void destruct(Server&, NodeId&) {}

    //
    // type life cycle
    //
    /*!
     * \brief typeConstructor
     * \param server
     * \param sessionId
     * \param sessionContext
     * \param typeNodeId
     * \param typeNodeContext
     * \param nodeId
     * \param nodeContext
     * \return error code
     */
    static UA_StatusCode typeConstructor(UA_Server* server,
                                         const UA_NodeId* sessionId,
                                         void* sessionContext,
                                         const UA_NodeId* typeNodeId,
                                         void* typeNodeContext,
                                         const UA_NodeId* nodeId,
                                         void** nodeContext);

    /* Can be NULL. May replace the nodeContext. */
    /*!
     * \brief typeDestructor
     * \param server
     * \param sessionId
     * \param sessionContext
     * \param typeNodeId
     * \param typeNodeContext
     * \param nodeId
     * \param nodeContext
     */
    static void typeDestructor(UA_Server* server,
                               const UA_NodeId* sessionId,
                               void* sessionContext,
                               const UA_NodeId* typeNodeId,
                               void* typeNodeContext,
                               const UA_NodeId* nodeId,
                               void** nodeContext);

    /*!
     * \brief typeConstruct
     * \return true on success
     */
    virtual bool typeConstruct(Server& /*server*/, NodeId& /*n*/, NodeId& /*t*/) { return true; }

    /*!
     * \brief typeDestruct
     * \param server
     * \param n
     */
    virtual void typeDestruct(Server& /*server*/, NodeId& /*n*/, NodeId& /*t*/) {}

    /*!
     * \brief setTypeLifeCycle
     * \param server
     * \param n
     * \return true on success
     */
    bool setTypeLifeCycle(Server& server, NodeId& n);

    //
    // Set up the data and value callbacks
    //
    /*!
        \brief readData
        \param node
        \param range
        \param value
        \return true on success
    */
    virtual bool readData(Server& server, NodeId& node, const UA_NumericRange* range, UA_DataValue& value)
    {
        if (_readData)
            return _readData(server, node, range, value);
        return false;
    }

    /*!
        \brief writeData
        \param server
        \param node
        \param range
        \param value
        \return true on success
    */
    virtual bool writeData(Server& server, NodeId& node, const UA_NumericRange* range, const UA_DataValue& value)
    {
        if (_writeData)
            return _writeData(server, node, range, value);
        return false;
    }

    /*!
     * \brief setAsDataSource
     * \param server
     * \param n
     * \return true on success
     */
    bool setAsDataSource(Server& server, NodeId& n);

    /*!
        \brief readDataSource
        \param server
        \param sessionId
        \param sessionContext
        \param nodeId
        \param nodeContext
        \param includeSourceTimeStamp
        \param range
        \param value
        \return error code
    */
    static UA_StatusCode readDataSource(UA_Server* server,
                                        const UA_NodeId* sessionId,
                                        void* sessionContext,
                                        const UA_NodeId* nodeId,
                                        void* nodeContext,
                                        UA_Boolean includeSourceTimeStamp,
                                        const UA_NumericRange* range,
                                        UA_DataValue* value);

    /*!
        \brief writeDataSource
        \param server
        \param sessionId
        \param sessionContext
        \param nodeId
        \param nodeContext
        \param range
        \param value
        \return error code
    */
    static UA_StatusCode writeDataSource(UA_Server* server,
                                         const UA_NodeId* sessionId,
                                         void* sessionContext,
                                         const UA_NodeId* nodeId,
                                         void* nodeContext,
                                         const UA_NumericRange* range,
                                         const UA_DataValue* value);

    /*!
     * \brief setValueCallback
     * \param server
     * \param n
     * \return true on success
     */
    bool setValueCallback(Open62541::Server& server, NodeId& n);
    /*!
        \brief readValue
        \param node
    */
    virtual void readValue(Server& server, NodeId& node, const UA_NumericRange* range, const UA_DataValue* value)
    {
        if (_readValue)
            _readValue(server, node, range, value);
    }
    /*!
        \brief writeValue
        \param node
    */
    virtual void writeValue(Server& server, NodeId& node, const UA_NumericRange* range, const UA_DataValue& value)
    {
        if (_writeValue)
            _writeValue(server, node, range, value);
    }

    // Value Callbacks
    /*!
        \brief readValueCallback
        \param server
        \param sessionId
        \param sessionContext
        \param nodeid
        \param nodeContext
        \param range
        \param value
    */
    static void readValueCallback(UA_Server* server,
                                  const UA_NodeId* sessionId,
                                  void* sessionContext,
                                  const UA_NodeId* nodeid,
                                  void* nodeContext,
                                  const UA_NumericRange* range,
                                  const UA_DataValue* value);
    /*!
        \brief writeValueCallback
        \param server
        \param sessionId
        \param sessionContext
        \param nodeId
        \param nodeContext
        \param range
        \param data
    */
    static void writeValueCallback(UA_Server* server,
                                   const UA_NodeId* sessionId,
                                   void* sessionContext,
                                   const UA_NodeId* nodeId,
                                   void* nodeContext,
                                   const UA_NumericRange* range,
                                   const UA_DataValue* data);
};

/*!
 * \brief The RegisteredNodeContext class
 * Can be used to setup stock call backs
 */
class RegisteredNodeContext : public NodeContext
{
    typedef std::map<std::string, NodeContext*> NodeContextMap;  // map of contexts
    static NodeContextMap _map;  // map of registered contexts - typically a static instance is used to self register
public:
    /*!
     * \brief RegisteredNodeContext
     * \param n
     */
    RegisteredNodeContext(const std::string& n)
        : NodeContext(n)
    {
        _map[n] = this;  // self register
    }
    /*!
     * \brief ~RegisteredNodeContext
     */
    virtual ~RegisteredNodeContext()
    {
        _map.erase(name());  // deregister on delete
    }

    /*!
     * \brief findRef
     * \param s
     * \return
     */
    static NodeContext* findRef(const std::string& s) { return _map[s]; }
};

}  // namespace Open62541
#endif  // METHODCONTEXT_H
