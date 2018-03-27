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
