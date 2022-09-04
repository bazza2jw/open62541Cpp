#include "testmethod.h"
#include <iostream>
#include <open62541cpp/open62541server.h>

UA_StatusCode TestMethod::callback(Open62541::Server& server,
                                   const UA_NodeId* /*objectId*/,
                                   size_t /*inputSize*/,
                                   const UA_Variant* /*input*/,
                                   size_t /*outputSize*/,
                                   UA_Variant* /*output*/)
{

    /* set up event */
    Open62541::NodeId eventNodeId;
    server.setUpEvent(eventNodeId, eventType, "TestEvent", "TestEventServer");
    server.triggerEvent(eventNodeId, Open62541::NodeId::Server);
    std::cout << "Event Triggered" << std::endl;

    return UA_STATUSCODE_GOOD;
}

bool TestMethod::initialise(Open62541::Server& server)
{
    eventType.notNull();
    server.addNewEventType("TestEvent", eventType, "Example Event");
    std::cout << "Added Event Type Event Node " << Open62541::toString(eventType) << std::endl;
    return true;
}
