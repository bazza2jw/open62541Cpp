/*
    Copyright (C) 2017 -  B. J. Hill

    This file is part of open62541 C++ classes. open62541 C++ classes are free software: you can
    redistribute it and/or modify it under the terms of the Mozilla Public
    License v2.0 as stated in the LICENSE file provided with open62541.

    open62541 C++ classes are distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
    A PARTICULAR PURPOSE.
*/
#ifndef CLIENTSUBSCRIPTION_H
#define CLIENTSUBSCRIPTION_H
#include <open62541cpp/open62541objects.h>
#include <open62541cpp/monitoreditem.h>
namespace Open62541 {

/*!
    \brief The ClientSubscription class
    Encapsulates a client subscription
*/

typedef std::shared_ptr<MonitoredItem> MonitoredItemRef;

typedef std::map<unsigned, MonitoredItemRef> MonitoredItemMap;

class UA_EXPORT ClientSubscription
{
    Client& _client;  // owning client
    CreateSubscriptionRequest _settings;
    CreateSubscriptionResponse _response;
    //
    int _monitorId = 0;     // key monitor items by Id
    MonitoredItemMap _map;  // map of monitor items - these are monitored items owned by this subscription
    //
protected:
    UA_StatusCode _lastError = 0;
    /*!
        \brief deleteSubscriptionCallback
        \param subscriptionContext
    */
    static void deleteSubscriptionCallback(UA_Client* client, UA_UInt32 subId, void* subscriptionContext);

    /*!
        \brief statusChangeNotificationCallback
        \param subscriptionContext
        \param notification
    */

    static void statusChangeNotificationCallback(UA_Client* client,
                                                 UA_UInt32 subId,
                                                 void* subscriptionContext,
                                                 UA_StatusChangeNotification* notification);

public:
    /*!
        \brief ClientSubscription
        \param c
    */
    ClientSubscription(Client& c);
    /*!
        \brief ~ClientSubscription
        Only delete subscriptions from the client
    */
    virtual ~ClientSubscription();
    /*!
        \brief create
        \return true on success
    */
    bool create();

    /*!
        \brief client
        \return reference to owning client
    */
    Client& client() { return _client; }

    /*!
        \brief id
        \return subscription id
    */
    UA_UInt32 id() { return _response.get().subscriptionId; }

    /*!
        \brief deleteSubscriptionCallback
    */
    virtual void deleteSubscription() {}
    /*!
        \brief changeNotificationCallback
    */
    virtual void statusChangeNotification(UA_StatusChangeNotification* /*notification*/) {}
    /*!
        \brief settings
        \return reference to the request structure
    */
    UA_CreateSubscriptionRequest& settings() { return _settings; }
    /*!
        \brief response
        \return reference to subscription response
    */
    UA_CreateSubscriptionResponse& response() { return _response; }

    /*!
        \brief addMonitorItem
        \param m monitored item
        \return monitored item
    */
    unsigned addMonitorItem(MonitoredItemRef& m)
    {
        _monitorId++;
        _map[_monitorId] = std::move(m);
        return _monitorId;
    }

    /*!
        \brief deleteMonitorItem
        \param id Id of the monitored item (from addMonitorItem) to delete
    */

    void deleteMonitorItem(unsigned id)
    {
        if (_map.find(id) != _map.end()) {
            MonitoredItemRef& m = _map[id];
            m->remove();
            _map.erase(id);
        }
    }

    /*!
        \brief findMonitorItem
        \param id Id of monitored item
        \return pointer to MonitoredItem or null
    */
    MonitoredItem* findMonitorItem(unsigned id)
    {
        if (_map.find(id) != _map.end()) {
            MonitoredItemRef& m = _map[id];
            return m.get();
        }
        return nullptr;
    }

    /*!
        \brief Open62541::ClientSubscription::addMonitorNodeId
        \param f functor tp handle item update
        \param n node id
    */
    template <typename T = Open62541::MonitoredItemDataChange>
    unsigned addMonitorNodeId(monitorItemFunc f, NodeId& n)
    {
        unsigned ret = 0;
        auto pdc     = new T(f, *this);
        if (pdc->addDataChange(n)) {  // make it notify on data change
            Open62541::MonitoredItemRef mcd(pdc);
            ret = addMonitorItem(mcd);  // add to subscription set
        }
        else {
            delete pdc;
        }
        return ret;  // returns item id
    }

    /*!
        \brief Open62541::ClientSubscription::addEventMonitor
        \param f event handler functor
        \param n node id
        \param ef event filter
    */
    template <typename T = Open62541::MonitoredItemEvent>
    unsigned addEventMonitor(monitorEventFunc f, NodeId& n)
    {
        unsigned ret = 0;  // item id
        auto pdc     = new T(f, *this);
        if (pdc->addEvent(n)) {  // make it notify on data change
            Open62541::MonitoredItemRef mcd(pdc);
            ret = addMonitorItem(mcd);  // add to subscription set
        }
        else {
            delete pdc;
        }
        return ret;
    }
};

typedef std::unique_ptr<ClientSubscription> ClientSubscriptionPtr;
}  // namespace Open62541

#endif  // CLIENTSUBSCRIPTION_H
