#include "EventClient.h"

void EventClient::subscribe()
{
    if (addSubscription(eventSubId)) {
        Open62541::ClientSubscription* c = subscription(eventSubId);
        if (c) {
            _monitorId = c->addEventMonitor<MonitorEvent>(
                [](Open62541::ClientSubscription&, Open62541::VariantArray&) {
                    std::cerr << "MonitoredItemEvent" << std::endl;
                },
                Open62541::NodeId::Server);
            std::cerr << "Added Event Monitor Id = " << _monitorId;
        }
    }
    else {
        std::cerr << "Failed to create subscription" << std::endl;
    }
}
