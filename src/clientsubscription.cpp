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
  if(id())
  {
      _map.clear(); // delete all monitored items
      UA_Client_Subscriptions_deleteSingle(_client.client(), id());
  }
}
/*!
* \brief create
* \return
*/
bool Open62541::ClientSubscription::create()
{
   _response.get() = UA_Client_Subscriptions_create(_client.client(), _settings,
                              (void *)(this),
                              statusChangeNotificationCallback,
                              deleteSubscriptionCallback);
   _lastError =  _response.get().responseHeader.serviceResult;
   return _lastError == UA_STATUSCODE_GOOD;
}


/*!
 * \brief Open62541::ClientSubscription::addMonitorNodeId
 * \param f
 * \param n
 */
unsigned Open62541::ClientSubscription::addMonitorNodeId(monitorItemFunc f, NodeId &n)
{
    unsigned ret = 0;
    auto pdc = new Open62541::MonitoredItemDataChange(f,*this);
    if(pdc->addDataChange(n)) // make it notify on data change
    {
        Open62541::MonitoredItemRef mcd(pdc);
        ret = addMonitorItem(mcd); // add to subscription set
    }
    else
    {
        delete pdc;
    }
    return ret;
}

/*!
 * \brief Open62541::ClientSubscription::addEventMonitor
 * \param f
 * \param n
 * \param ef
 */
unsigned Open62541::ClientSubscription::addEventMonitor(monitorEventFunc f, NodeId &n, EventFilterSelect *ef)
{
    unsigned ret = 0;
    auto pdc = new Open62541::MonitoredItemEvent(f,*this);
    if(pdc->addEvent(n,ef)) // make it notify on data change
    {
        Open62541::MonitoredItemRef mcd(pdc);
        ret = addMonitorItem(mcd); // add to subscription set
    }
    else
    {
        delete pdc;
    }
    return ret;
}




