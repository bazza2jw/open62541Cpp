#ifndef EVENTCLIENT_H
#define EVENTCLIENT_H
#include <open62541cpp/open62541client.h>
#include <open62541cpp/clientsubscription.h>
#include <open62541cpp/monitoreditem.h>

class MonitorEvent : public Open62541::MonitoredItemEvent
{
public:
    MonitorEvent(Open62541::monitorEventFunc f, Open62541::ClientSubscription &s) : Open62541::MonitoredItemEvent(f,s)
    {
        setMonitorItem(Open62541::NodeId::Server, 2); // initialise the event monitor
        setClause(0,"Message");
        setClause(1,"Severity");
    }
};

class EventClient : public Open62541::Client
{
    UA_UInt32 eventSubId = 0;
    unsigned _monitorId = 0;
public:
    EventClient()
    {
    }
    void subscribe();
};


#endif // EVENTCLIENT_H
