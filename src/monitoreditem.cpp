/*
    Copyright (C) 2017 -  B. J. Hill

    This file is part of open62541 C++ classes. open62541 C++ classes are free software: you can
    redistribute it and/or modify it under the terms of the Mozilla Public
    License v2.0 as stated in the LICENSE file provided with open62541.

    open62541 C++ classes are distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
    A PARTICULAR PURPOSE.
*/
#include <open62541cpp/monitoreditem.h>
#include <open62541cpp/open62541client.h>
#include <open62541cpp/clientsubscription.h>

/*!
    \brief Open62541::MonitoredItem::MonitoredItem
    \param s
*/
Open62541::MonitoredItem::MonitoredItem(ClientSubscription& s)
    : _sub(s)
{
}

/* Callback for the deletion of a MonitoredItem */
/* any of the parts may have disappeared */
/*!
    \brief Open62541::MonitoredItem::deleteMonitoredItemCallback
    \param client
    \param subId
    \param subContext
    \param monId
    \param monContext
*/
void Open62541::MonitoredItem::deleteMonitoredItemCallback(UA_Client* client,
                                                           UA_UInt32 subId,
                                                           void* /*subContext*/,
                                                           UA_UInt32 /*monId*/,
                                                           void* monContext)
{
    //
    // The subscription
    Client* cl = (Client*)UA_Client_getContext(client);
    if (cl && (cl->getConnectStatus() == UA_STATUSCODE_GOOD)) {
        ClientSubscription* c = cl->subscription(subId);
        if (c) {
            Open62541::MonitoredItem* m = (Open62541::MonitoredItem*)(monContext);
            if (m) {
                m->deleteMonitoredItem();
            }
        }
    }
}

/* Callback for DataChange notifications */
/*!
    \brief Open62541::MonitoredItem::dataChangeNotificationCallback
    \param client
    \param subId
    \param subContext
    \param monId
    \param monContext
    \param value
*/
void Open62541::MonitoredItem::dataChangeNotificationCallback(UA_Client* client,
                                                              UA_UInt32 subId,
                                                              void* /*subContext*/,
                                                              UA_UInt32 /*monId*/,
                                                              void* monContext,
                                                              UA_DataValue* value)
{
    Client* cl = (Client*)UA_Client_getContext(client);
    if (cl && (cl->getConnectStatus() == UA_STATUSCODE_GOOD)) {
        ClientSubscription* c = cl->subscription(subId);
        if (c) {
            Open62541::MonitoredItem* m = (Open62541::MonitoredItem*)(monContext);
            if (m) {
                m->dataChangeNotification(value);
            }
        }
    }
}

/* Callback for Event notifications */
/*!
    \brief Open62541::MonitoredItem::eventNotificationCallback
    \param client
    \param subId
    \param subContext
    \param monId
    \param monContext
    \param nEventFields
    \param eventFields
*/
void Open62541::MonitoredItem::eventNotificationCallback(UA_Client* client,
                                                         UA_UInt32 subId,
                                                         void* /*subContext*/,
                                                         UA_UInt32 /*monId*/,
                                                         void* monContext,
                                                         size_t nEventFields,
                                                         UA_Variant* eventFields)
{
    Client* cl = (Client*)UA_Client_getContext(client);
    if (cl && (cl->getConnectStatus() == UA_STATUSCODE_GOOD)) {
        ClientSubscription* c = cl->subscription(subId);
        if (c) {
            Open62541::MonitoredItem* m = (Open62541::MonitoredItem*)(monContext);
            if (m) {
                m->eventNotification(nEventFields, eventFields);
            }
        }
    }
}

/*!
    \brief Open62541::MonitoredItem::remove
    \return
*/
bool Open62541::MonitoredItem::remove()
{
    bool ret = false;
    if ((id() > 0) && _sub.client().client()) {
        ret = UA_Client_MonitoredItems_deleteSingle(_sub.client().client(), _sub.id(), id()) == UA_STATUSCODE_GOOD;
        _response.null();
    }
    return ret;
}

/*!
 * \brief setMonitoringMode
 * \param request
 * \param response
 * \return
 */
bool Open62541::MonitoredItem::setMonitoringMode(const SetMonitoringModeRequest& request,
                                                 SetMonitoringModeResponse& response)
{
    response.get() = UA_Client_MonitoredItems_setMonitoringMode(subscription().client().client(), request.get());
    return true;
}

/*!
 * \brief setTriggering
 * \param request
 * \param request
 * \return
 */
bool Open62541::MonitoredItem::setTriggering(const SetTriggeringRequest& request, SetTriggeringResponse& response)
{
    response.get() = UA_Client_MonitoredItems_setTriggering(subscription().client().client(), request.get());
    return true;
}

/*!
    \brief Open62541::MonitoredItem::addDataChange
    \param n
    \return
*/
bool Open62541::MonitoredItemDataChange::addDataChange(NodeId& n, UA_TimestampsToReturn ts)
{
    MonitoredItemCreateRequest monRequest;
    monRequest      = UA_MonitoredItemCreateRequest_default(n);
    _response.get() = UA_Client_MonitoredItems_createDataChange(subscription().client().client(),
                                                                subscription().id(),
                                                                ts,
                                                                monRequest,
                                                                this,
                                                                dataChangeNotificationCallback,
                                                                deleteMonitoredItemCallback);
    return _response.get().statusCode == UA_STATUSCODE_GOOD;
}

/*!
    \brief addEvent
    \param n
    \param ts
    \return
*/
bool Open62541::MonitoredItemEvent::addEvent(NodeId& n, UA_TimestampsToReturn ts)
{
    remove();  // delete any existing item

    _response = UA_Client_MonitoredItems_createEvent(subscription().client().client(),
                                                     subscription().id(),
                                                     ts,
                                                     _monitorItem,
                                                     this,
                                                     eventNotificationCallback,
                                                     deleteMonitoredItemCallback);
    return _response.get().statusCode == UA_STATUSCODE_GOOD;
}
