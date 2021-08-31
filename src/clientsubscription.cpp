/*
    Copyright (C) 2017 -  B. J. Hill

    This file is part of open62541 C++ classes. open62541 C++ classes are free software: you can
    redistribute it and/or modify it under the terms of the Mozilla Public
    License v2.0 as stated in the LICENSE file provided with open62541.

    open62541 C++ classes are distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
    A PARTICULAR PURPOSE.
*/
#include <open62541cpp/clientsubscription.h>
#include <open62541cpp/open62541client.h>

void Open62541::ClientSubscription::deleteSubscriptionCallback(UA_Client* client,
                                                               UA_UInt32 subId,
                                                               void* subscriptionContext)
{
    Client* cl = (Client*)UA_Client_getContext(client);
    if (cl) {
        ClientSubscription* c = cl->subscription(subId);
        if (c)
            c->deleteSubscription();
    }
}

/*!
    \brief statusChangeNotificationCallback
    \param subscriptionContext
    \param notification
*/

void Open62541::ClientSubscription::statusChangeNotificationCallback(UA_Client* client,
                                                                     UA_UInt32 subId,
                                                                     void* /*subscriptionContext*/,
                                                                     UA_StatusChangeNotification* notification)
{
    Client* cl = (Client*)UA_Client_getContext(client);
    if (cl) {
        ClientSubscription* c = cl->subscription(subId);
        if (c)
            c->statusChangeNotification(notification);
    }
}

Open62541::ClientSubscription::ClientSubscription(Client& c)
    : _client(c)
{
    _settings.get() = UA_CreateSubscriptionRequest_default();
}
/*!
    \brief ~ClientSubscription
*/
Open62541::ClientSubscription::~ClientSubscription()
{
    if (id()) {
        _map.clear();  // delete all monitored items
        if (_client.client())
            UA_Client_Subscriptions_deleteSingle(_client.client(), id());
    }
}
/*!
    \brief create
    \return true on success
*/
bool Open62541::ClientSubscription::create()
{
    if (_client.client()) {
        _response.get() = UA_Client_Subscriptions_create(_client.client(),
                                                         _settings,
                                                         (void*)(this),
                                                         statusChangeNotificationCallback,
                                                         deleteSubscriptionCallback);
        _lastError      = _response.get().responseHeader.serviceResult;
        return _lastError == UA_STATUSCODE_GOOD;
    }
    return false;
}
