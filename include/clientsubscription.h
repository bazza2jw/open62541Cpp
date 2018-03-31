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
#ifndef CLIENTSUBSCRIPTION_H
#define CLIENTSUBSCRIPTION_H
#include <open62541objects.h>

namespace Open62541 {

/*!
    \brief The ClientSubscription class
*/
class UA_EXPORT ClientSubscription {
        Client &_client;
        CreateSubscriptionRequest _settings;
        CreateSubscriptionResponse _response;


    protected:
        UA_StatusCode _lastError = 0;
        /*!
         * \brief subscriptionInactivityCallback
         * \param subscriptionContext
         */
        static void  subscriptionInactivityCallback(UA_Client *, UA_UInt32 , void *subscriptionContext,
                                                    UA_StatusChangeNotification *notification)
        {
            ClientSubscription * p = (ClientSubscription *)(subscriptionContext);
            if(p) p->subscriptionInactivity(notification);
        }

        /*!
         * \brief deleteSubscriptionCallback
         * \param subscriptionContext
         */
        static void  deleteSubscriptionCallback(UA_Client *, UA_UInt32 , void *subscriptionContext)
        {
            ClientSubscription * p = (ClientSubscription *)(subscriptionContext);
            if(p)p->deleteSubscriptionCallback();
        }

    public:
        /*!
         * \brief ClientSubscription
         * \param c
         */
        ClientSubscription(Client &c);
        /*!
         * \brief ~ClientSubscription
         */
        virtual ~ClientSubscription();
        /*!
         * \brief create
         * \return
         */
        bool create();

        Client & client() { return _client;}

        /*!
         * \brief id
         * \return
         */
        UA_UInt32 id()
        {
            return _response.get().subscriptionId;
        }

        virtual void  subscriptionInactivity(UA_StatusChangeNotification */*notification*/){}
        virtual void  deleteSubscriptionCallback(){}
        UA_CreateSubscriptionRequest & settings() { return  _settings;}
        UA_CreateSubscriptionResponse & response() { return _response;}
};

typedef std::shared_ptr<ClientSubscription> ClientSubscriptionRef;

}



#endif // CLIENTSUBSCRIPTION_H
