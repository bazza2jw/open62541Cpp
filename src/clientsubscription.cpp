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
#include <clientsubscription.h>
#include <open62541client.h>

Open62541::ClientSubscription::ClientSubscription(Client &c) : _client(c)
{
   _settings.get() = UA_CreateSubscriptionRequest_default();

}
/*!
* \brief ~ClientSubscription
*/
Open62541::ClientSubscription::~ClientSubscription()
{
  if(id()) UA_Client_Subscriptions_deleteSingle(_client.client(), id());
}
/*!
* \brief create
* \return
*/
bool Open62541::ClientSubscription::create()
{
   _response.get() = UA_Client_Subscriptions_create(_client.client(), _settings,
                              (void *)(this),subscriptionInactivityCallback,
                              deleteSubscriptionCallback);
   _lastError =  _response.get().responseHeader.serviceResult;
   return _lastError == UA_STATUSCODE_GOOD;
}
