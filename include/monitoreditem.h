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
#include <open62541objects.h>

namespace  Open62541 {


    class ClientSubscription;

    typedef std::function<void (ClientSubscription &, UA_DataValue *)> monitorItemFunc;
    typedef std::function<void (ClientSubscription &, VariantArray &)> monitorEventFunc;
    /*!
        \brief The MonitoredItem class
    */
    class  UA_EXPORT  MonitoredItem {
            ClientSubscription &_sub; // parent subscription
        protected:
            MonitoredItemCreateResult _response; // response
            UA_StatusCode _lastError = 0;

            /* Callback for the deletion of a MonitoredItem */
            static void deleteMonitoredItemCallback
            (UA_Client *client, UA_UInt32 subId, void *subContext,
             UA_UInt32 monId, void *monContext);

            /* Callback for DataChange notifications */
            static  void dataChangeNotificationCallback
            (UA_Client *client, UA_UInt32 subId, void *subContext,
             UA_UInt32 monId, void *monContext,
             UA_DataValue *value);

            /* Callback for Event notifications */
            static void eventNotificationCallback
            (UA_Client *client, UA_UInt32 subId, void *subContext,
             UA_UInt32 monId, void *monContext,
             size_t nEventFields, UA_Variant *eventFields);

        public:
            /*!
                \brief MonitoredItem
                \param s
            */
            MonitoredItem(ClientSubscription &s);

            /*!
                \brief ~MonitoredItem
            */
            virtual ~MonitoredItem() {
                remove();
            }
            /*!
                \brief lastError
                \return
            */
            UA_StatusCode  lastError() const {
                return _lastError;
            }

            /*!
             * \brief subscription
             * \return
             */
            ClientSubscription &subscription() { return _sub;} // parent subscription

            //
            // Notification handlers
            //
            /*!
                \brief deleteMonitoredItem
            */
            virtual void deleteMonitoredItem() {remove();}
            /*!
                \brief dataChangeNotification
                \param value
            */
            virtual void dataChangeNotification(UA_DataValue *) {}
            /*!
             * \brief eventNotification
             * \param nEventFields
             * \param eventFields
             */
            virtual void eventNotification(size_t /*nEventFields*/, UA_Variant * /*eventFields*/){}
            //
            /*!
                \brief remove
                \return
            */
            virtual bool remove();
            /*!
                \brief id
                \return
            */
            UA_UInt32 id() {
                return _response.get().monitoredItemId;
            }
    };

    /*!
        \brief The MonitoredItemDataChange class
    */
    class MonitoredItemDataChange : public MonitoredItem {
            monitorItemFunc _func; // lambda for callback

        public:
            /*!
                \brief MonitoredItem
                \param s
            */
            MonitoredItemDataChange(ClientSubscription &s) : MonitoredItem(s) {}
            /*!
                \brief MonitoredItem
                \param f
                \param s
            */
            MonitoredItemDataChange(monitorItemFunc f, ClientSubscription &s) : MonitoredItem(s), _func(f) {}
            /*!
                \brief setFunction
                \param f
            */
            void setFunction(monitorItemFunc f) {
                _func = f;
            }
            /*!
                \brief dataChangeNotification
                \param value
            */
            virtual void dataChangeNotification(UA_DataValue *value) {
                if (_func) _func(subscription(), value); // invoke functor
            }

            /*!
                \brief addDataChange
                \param n
                \param ts
                \return
            */
            bool addDataChange(NodeId &n, UA_TimestampsToReturn ts = UA_TIMESTAMPSTORETURN_BOTH);
    };

    /*!
        \brief The MonitoredItemEvent class
    */
    class MonitoredItemEvent : public MonitoredItem {
            monitorEventFunc _func; // the event call functor
            EventFilterSelect * _events = nullptr;
        public:
            /*!
                \brief MonitoredItem
                \param s
            */
            MonitoredItemEvent(ClientSubscription &s) : MonitoredItem(s) {}
            /*!
                \brief MonitoredItem
                \param f
                \param s
            */
            MonitoredItemEvent(monitorEventFunc f, ClientSubscription &s) : MonitoredItem(s), _func(f) {}


            /*!
             * \brief remove
             * \return
             */
            bool remove()
            {
                bool ret = MonitoredItem::remove();
                if(_events) delete _events;
                return ret;
            }


            /*!
                \brief setFunction
                \param f
            */
            void setFunction(monitorEventFunc f) {
                _func = f;
            }

            /*!
                \brief eventNotification
            */
            virtual void eventNotification(size_t nEventFields, UA_Variant *eventFields) {
                if (_func) {
                    VariantArray va;
                    va.setList(nEventFields, eventFields);
                    _func(subscription(), va); // invoke functor
                    va.release();
                }
            }

            /*!
                \brief addEvent
                \param n
                \param ts
                \return
            */
            bool addEvent(NodeId &n,  EventFilterSelect *events, UA_TimestampsToReturn ts = UA_TIMESTAMPSTORETURN_BOTH);


    };
}


#endif // MONITOREDITEM_H
