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
#include "open62541objects.h"
#include "monitoreditem.h"

namespace Open62541 {

    /*!
        \brief The ClientSubscription class
    */

    typedef std::shared_ptr<MonitoredItem> MonitoredItemRef;

    typedef std::map<unsigned, MonitoredItemRef> MonitoredItemMap;

    class UA_EXPORT ClientSubscription {
            Client &_client;  // owning client
            CreateSubscriptionRequest _settings;
            CreateSubscriptionResponse _response;
            //
            int _monitorId = 0; // key monitor items by Id
            MonitoredItemMap _map; // map of monitor items - these are monitored items owned by this subscription
            //
        protected:
            UA_StatusCode _lastError = 0;
            /*!
                \brief deleteSubscriptionCallback
                \param subscriptionContext
            */
            static void  deleteSubscriptionCallback(UA_Client *, UA_UInt32, void *subscriptionContext) {
                ClientSubscription *p = (ClientSubscription *)(subscriptionContext);
                if (p)p->deleteSubscription();
            }

            /*!
                \brief statusChangeNotificationCallback
                \param subscriptionContext
                \param notification
            */

            static void statusChangeNotificationCallback(UA_Client * /*client*/, UA_UInt32 /*subId*/, void *subscriptionContext,
                                                         UA_StatusChangeNotification *notification) {
                ClientSubscription *p = (ClientSubscription *)(subscriptionContext);
                if (p)p->statusChangeNotification(notification);
            }

        public:
            /*!
                \brief ClientSubscription
                \param c
            */
            ClientSubscription(Client &c);
            /*!
                \brief ~ClientSubscription
                Only delete subscriptions from the client
            */
            virtual ~ClientSubscription();
            /*!
                \brief create
                \return
            */
            bool create();

            /*!
                \brief client
                \return
            */
            Client &client() {
                return _client;
            }

            /*!
                \brief id
                \return
            */
            UA_UInt32 id() {
                return _response.get().subscriptionId;
            }

            /*!
                \brief deleteSubscriptionCallback
            */
            virtual void  deleteSubscription() {}
            /*!
                \brief changeNotificationCallback
            */
            virtual void  statusChangeNotification(UA_StatusChangeNotification * /*notification*/) {}
            /*!
                \brief settings
                \return
            */
            UA_CreateSubscriptionRequest &settings() {
                return  _settings;
            }
            /*!
                \brief response
                \return
            */
            UA_CreateSubscriptionResponse &response() {
                return _response;
            }

            /*!
                \brief addMonitorItem
                \param m
            */
            unsigned addMonitorItem(MonitoredItemRef &m) {
                _monitorId++;
                _map[_monitorId] = m;
                return _monitorId;
            }

            /*!
                \brief deleteMonitorItem
                \param id
            */

            void deleteMonitorItem(unsigned id) {
                if (_map.find(id) != _map.end()) {
                    MonitoredItemRef &m = _map[id];
                    m->remove();
                    _map.erase(id);
                }
            }

            /*!
                \brief findMonitorItem
                \param id
                \return
            */
            MonitoredItem *findMonitorItem(unsigned id) {
                if (_map.find(id) != _map.end()) {
                    MonitoredItemRef &m = _map[id];
                    return m.get();
                }
                return nullptr;
            }

            /*!
             * \brief addMonitorNodeId
             * \param n
             */
            unsigned addMonitorNodeId(monitorItemFunc f, NodeId &n);
            /*!
             * \brief addEventMonitor
             * \param f
             * \param n
             */
            unsigned addEventMonitor(monitorEventFunc f, NodeId &n, Open62541::EventFilterSelect *ef);

    };
}



#endif // CLIENTSUBSCRIPTION_H
