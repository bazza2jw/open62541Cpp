/*
    Copyright (C) 2017 -  B. J. Hill

    This file is part of open62541 C++ classes. open62541 C++ classes are free software: you can
    redistribute it and/or modify it under the terms of the Mozilla Public
    License v2.0 as stated in the LICENSE file provided with open62541.

    open62541 C++ classes are distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
    A PARTICULAR PURPOSE.
*/
#include <monitoreditem.h>
#include <open62541client.h>
#include <clientsubscription.h>

/*!
    \brief Open62541::MonitoredItem::MonitoredItem
    \param s
*/
Open62541::MonitoredItem::MonitoredItem(ClientSubscription &s) : _sub(s) {

}


/* Callback for the deletion of a MonitoredItem */
/*!
    \brief Open62541::MonitoredItem::deleteMonitoredItemCallback
    \param client
    \param subId
    \param subContext
    \param monId
    \param monContext
*/
void Open62541::MonitoredItem::deleteMonitoredItemCallback
(UA_Client * /*client*/, UA_UInt32 /*subId*/, void *subContext,
 UA_UInt32 /*monId*/, void *monContext) {
    Open62541::MonitoredItem *m = (Open62541::MonitoredItem *)(monContext);
    Open62541::ClientSubscription *c = (Open62541::ClientSubscription *)subContext;
    if (m && c) {
        m->deleteMonitoredItem();
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
void Open62541::MonitoredItem::dataChangeNotificationCallback
(UA_Client * /*client*/, UA_UInt32 /*subId*/, void *subContext,
 UA_UInt32 /*monId*/, void *monContext,
 UA_DataValue *value) {
    Open62541::MonitoredItem *m = (Open62541::MonitoredItem *)(monContext);
    Open62541::ClientSubscription *c = (Open62541::ClientSubscription *)subContext;
    if (m && c) {
        m->dataChangeNotification(value);
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
void Open62541::MonitoredItem::eventNotificationCallback
(UA_Client * /*client*/, UA_UInt32 /*subId*/, void *subContext,
 UA_UInt32 /*monId*/, void *monContext,
 size_t nEventFields, UA_Variant *eventFields) {
    Open62541::MonitoredItem *m = (Open62541::MonitoredItem *)(monContext);
    Open62541::ClientSubscription *c = (Open62541::ClientSubscription *)subContext;
    if (m && c) {
        m->eventNotification(nEventFields, eventFields);
    }
}

/*!
    \brief Open62541::MonitoredItem::remove
    \return
*/
bool  Open62541::MonitoredItem::remove() {
    bool ret =  false;
    if ((id() > 0) && _sub.client().client() ) {
        ret = UA_Client_MonitoredItems_deleteSingle(_sub.client().client(), _sub.id(), id()) == UA_STATUSCODE_GOOD;
        _response.null();
    }
    return ret;
}

/*!
    \brief Open62541::MonitoredItem::addDataChange
    \param n
    \return
*/
bool Open62541::MonitoredItemDataChange::addDataChange(NodeId &n, UA_TimestampsToReturn ts) {
    MonitoredItemCreateRequest monRequest;
    monRequest = UA_MonitoredItemCreateRequest_default(n);
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
bool Open62541::MonitoredItemEvent::addEvent(NodeId &n, EventFilterSelect *events, UA_TimestampsToReturn ts) {
    if (events) {
        //
        remove(); // delete any existing item
        //
        _events = events; // take ownership - events must be deleted after the item is removed
        MonitoredItemCreateRequest item;
        item = UA_MonitoredItemCreateRequest_default(n);

        item.get().itemToMonitor.nodeId = n;
        item.get().itemToMonitor.attributeId = UA_ATTRIBUTEID_EVENTNOTIFIER;
        item.get().monitoringMode = UA_MONITORINGMODE_REPORTING;

        item.get().requestedParameters.filter.encoding = UA_EXTENSIONOBJECT_DECODED;
        item.get().requestedParameters.filter.content.decoded.data = events->ref();
        item.get().requestedParameters.filter.content.decoded.type = &UA_TYPES[UA_TYPES_EVENTFILTER];

        _response = UA_Client_MonitoredItems_createEvent(subscription().client().client(),
                                                         subscription().id(),
                                                         ts,
                                                         item,
                                                         this,
                                                         eventNotificationCallback,
                                                         deleteMonitoredItemCallback);
        return _response.get().statusCode == UA_STATUSCODE_GOOD;
    }
    return false;
}


