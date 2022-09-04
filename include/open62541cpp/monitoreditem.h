/*
    Copyright (C) 2017 -  B. J. Hill

    This file is part of open62541 C++ classes. open62541 C++ classes are free software: you can
    redistribute it and/or modify it under the terms of the Mozilla Public
    License v2.0 as stated in the LICENSE file provided with open62541.

    open62541 C++ classes are distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
    A PARTICULAR PURPOSE.
*/
#ifndef MONITOREDITEM_H
#define MONITOREDITEM_H
#include <open62541cpp/open62541objects.h>

namespace Open62541 {

class ClientSubscription;

// Callback for a (data change)  monitored item
class MonitoredItemEvent;
typedef std::function<void(ClientSubscription&, MonitoredItem *,  UA_DataValue*)> monitorItemFunc;
// call back for an event
typedef std::function<void(ClientSubscription&, MonitoredItemEvent *, VariantArray&)> monitorEventFunc;
/*!
    \brief The MonitoredItem class
    This is a single monitored event. Monitored events are associated (owned) by subscriptions
*/
class UA_EXPORT MonitoredItem
{
private:
    ClientSubscription& _sub;  // parent subscription
protected:
    MonitoredItemCreateResult _response;  // response
    UA_StatusCode _lastError = 0;

    /* Callback for the deletion of a MonitoredItem */
    static void deleteMonitoredItemCallback(UA_Client* client,
                                            UA_UInt32 subId,
                                            void* subContext,
                                            UA_UInt32 monId,
                                            void* monContext);

    /* Callback for DataChange notifications */
    static void dataChangeNotificationCallback(UA_Client* client,
                                               UA_UInt32 subId,
                                               void* subContext,
                                               UA_UInt32 monId,
                                               void* monContext,
                                               UA_DataValue* value);

    /* Callback for Event notifications */
    static void eventNotificationCallback(UA_Client* client,
                                          UA_UInt32 subId,
                                          void* subContext,
                                          UA_UInt32 monId,
                                          void* monContext,
                                          size_t nEventFields,
                                          UA_Variant* eventFields);

public:
    /*!
        \brief MonitoredItem
        \param s owning subscription
    */
    MonitoredItem(ClientSubscription& s);

    /*!
        \brief ~MonitoredItem
    */
    virtual ~MonitoredItem() { remove(); }
    /*!
        \brief lastError
        \return last error code
    */
    UA_StatusCode lastError() const { return _lastError; }

    /*!
     * \brief subscription
     * \return owning subscription
     */
    ClientSubscription& subscription() { return _sub; }  // parent subscription

    //
    // Notification handlers
    //

    /*!
        \brief deleteMonitoredItem
    */
    virtual void deleteMonitoredItem() { remove(); }
    /*!
        \brief dataChangeNotification
        \param value
    */
    virtual void dataChangeNotification(UA_DataValue*) {}
    /*!
     * \brief eventNotification
     * \param nEventFields
     * \param eventFields
     */
    virtual void eventNotification(size_t /*nEventFields*/, UA_Variant* /*eventFields*/) {}
    //
    /*!
        \brief remove
        \return true on success
    */
    virtual bool remove();
    /*!
        \brief id
        \return the id of the monitored event
    */
    UA_UInt32 id() { return _response.get().monitoredItemId; }

protected:
    /*!
     * \brief setMonitoringMode
     * \param request
     * \param response
     * \return
     */
    bool setMonitoringMode(const SetMonitoringModeRequest& request, SetMonitoringModeResponse& response);

    /*!
     * \brief setTriggering
     * \param request
     * \param request
     * \return
     */
    bool setTriggering(const SetTriggeringRequest& request, SetTriggeringResponse& response);
};

/*!
    \brief The MonitoredItemDataChange class
    Handles value change notifications
*/
class MonitoredItemDataChange : public MonitoredItem
{
    monitorItemFunc _func;  // lambda for callback

public:
    /*!
        \brief MonitoredItem
        \param s owning subscription
    */
    MonitoredItemDataChange(ClientSubscription& s)
        : MonitoredItem(s)
    {
    }
    /*!
        \brief MonitoredItem
        \param f functor to handle notifications
        \param s owning subscription
    */
    MonitoredItemDataChange(monitorItemFunc f, ClientSubscription& s)
        : MonitoredItem(s)
        , _func(f)
    {
    }
    /*!
        \brief setFunction
        \param f functor
    */
    void setFunction(monitorItemFunc f) { _func = f; }
    /*!
        \brief dataChangeNotification
        \param value new value
    */
    virtual void dataChangeNotification(UA_DataValue* value)
    {
        if (_func)
            _func(subscription(),this, value);  // invoke functor
    }

    /*!
        \brief addDataChange
        \param n node id
        \param ts timestamp specification
        \return true on success
    */
    bool addDataChange(NodeId& n, UA_TimestampsToReturn ts = UA_TIMESTAMPSTORETURN_BOTH);
};

typedef std::unique_ptr<MonitoredItem> MonitoredItemPtr;
/*!
    \brief The MonitoredItemEvent class
*/
class MonitoredItemEvent : public MonitoredItem
{
    monitorEventFunc _func;                   // the event call functor
    MonitoredItemCreateRequest _monitorItem;  // must persist
public:
    /*!
        \brief MonitoredItem
        \param s owning subscription
    */
    MonitoredItemEvent(ClientSubscription& s)
        : MonitoredItem(s)
    {
    }
    /*!
        \brief MonitoredItem
        \param f functor to handle event notifications
        \param s owning subscriptions
    */
    MonitoredItemEvent(monitorEventFunc f, ClientSubscription& s)
        : MonitoredItem(s)
        , _func(f)
    {
    }

    /*!
     * \brief remove
     * \return true on success
     */
    bool remove()
    {
        bool ret = MonitoredItem::remove();
        return ret;
    }

    /*!
        \brief setFunction
        \param f functor
    */
    void setFunction(monitorEventFunc f) { _func = f; }

    /*!
        \brief eventNotification
        Handles the event notification
    */
    virtual void eventNotification(size_t nEventFields, UA_Variant* eventFields)
    {
        if (_func) {
            VariantArray va;
            va.setList(nEventFields, eventFields);
            _func(subscription(),this, va);  // invoke functor
            va.release();
        }
    }

    /*!
        \brief addEvent
        \param n node id
        \param events event filter
        \param ts timestamp flags
        \return true on success
    */
    virtual bool addEvent(NodeId& n, UA_TimestampsToReturn ts = UA_TIMESTAMPSTORETURN_BOTH);

    /*!
     * \brief monitorItem
     * \return
     */
    MonitoredItemCreateRequest& monitorItem() { return _monitorItem; }

    /*!
     * \brief setItem
     * \param nodeId
     * \param filter
     */
    void setMonitorItem(const Open62541::NodeId& nodeId, size_t nSelect)
    {
        // set up defaults - ownership is tricky with this one - there be dragons!
        UA_SimpleAttributeOperand* selectClauses =
            (UA_SimpleAttributeOperand*)UA_Array_new(nSelect, &UA_TYPES[UA_TYPES_SIMPLEATTRIBUTEOPERAND]);
        UA_EventFilter* f = new UA_EventFilter;
        UA_EventFilter_init(f);
        f->selectClauses     = selectClauses;
        f->selectClausesSize = nSelect;
        //
        _monitorItem.null();  // clear it
        _monitorItem.setItem(nodeId);
        _monitorItem.setFilter(f);  // this object owns the filter now
    }

    /*!
     * \brief setClause
     * \param i
     * \param browsePath
     * \param attributeId
     * \param typeDefintion
     * \param indexRange
     */
    void setClause(size_t i,
                   const std::string& browsePath,
                   UA_UInt32 attributeId         = UA_ATTRIBUTEID_VALUE,
                   const NodeId& typeDefintion   = NodeId::BaseEventType,
                   const std::string& indexRange = "")
    {
        UA_EventFilter* f = _monitorItem.filter();
        if (f && (i < f->selectClausesSize)) {
            UA_SimpleAttributeOperand_init(f->selectClauses + i);
            UA_SimpleAttributeOperand& a = f->selectClauses[i];
            a.typeDefinitionId           = typeDefintion;
            a.browsePathSize             = 1;
            a.browsePath                 = (UA_QualifiedName*)UA_Array_new(1, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
            a.attributeId                = attributeId;
            a.browsePath[0]              = UA_QUALIFIEDNAME_ALLOC(0, browsePath.c_str());
            a.indexRange                 = UA_STRING_ALLOC(indexRange.c_str());
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
    void setClause(size_t i,
                   StdStringArray& browsePath,
                   UA_UInt32 attributeId         = UA_ATTRIBUTEID_VALUE,
                   const NodeId& typeDefintion   = NodeId::BaseEventType,
                   const std::string& indexRange = "")
    {
        UA_EventFilter* f = _monitorItem.filter();
        if (f && (i < f->selectClausesSize) && (browsePath.size() > 0)) {
            UA_SimpleAttributeOperand_init(f->selectClauses + i);
            UA_SimpleAttributeOperand& a = f->selectClauses[i];
            a.typeDefinitionId           = typeDefintion;
            a.browsePathSize             = browsePath.size();
            a.browsePath  = (UA_QualifiedName*)UA_Array_new(a.browsePathSize, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
            a.attributeId = attributeId;
            a.indexRange  = UA_STRING_ALLOC(indexRange.c_str());
            for (size_t j = 0; j < a.browsePathSize; j++) {
                a.browsePath[j] = UA_QUALIFIEDNAME_ALLOC(0, browsePath[j].c_str());
            }
        }
    }
};
typedef std::unique_ptr<MonitoredItemEvent> MonitoredItemEventPtr;
}  // namespace Open62541

#endif  // MONITOREDITEM_H
