#ifndef MONITOREDITEM_H
#define MONITOREDITEM_H
#include <open62541objects.h>

namespace  Open62541
{

typedef std::function<void (ClientSubscription &, UA_DataValue *)> monitorItemFunc;
/*!
    \brief The MonitoredItem class
*/
class  UA_EXPORT  MonitoredItem {
        MonitoredItemCreateResult _response;
        monitorItemFunc _func; // lambda for callback
        ClientSubscription &_sub;
    protected:
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

        MonitoredItem(ClientSubscription &s);
        /*!
            \brief ~MonitoredItem
        */
        virtual ~MonitoredItem()
        {
            remove();
        }

        UA_StatusCode  lastError() const {return _lastError; }
        //
        // Notification handlers
        //
        virtual void deleteMonitoredItem( )
        {
            remove();
        }
        virtual void dataChangeNotification( UA_DataValue *value)
        {
            if(_func) _func(_sub,value); // invoke functor
        }
        virtual void eventNotification( size_t /*nEventFields*/, UA_Variant */*eventFields*/){}
        //
        bool addDataChange( NodeId &n, UA_TimestampsToReturn ts = UA_TIMESTAMPSTORETURN_BOTH);
        bool remove();
        UA_UInt32 id()
        {
            return _response.get().monitoredItemId;
        }

        // TO DO Add Event handling

};

typedef std::shared_ptr<MonitoredItem> MonitoredItemRef;

}


#endif // MONITOREDITEM_H
