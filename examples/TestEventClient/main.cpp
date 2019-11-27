#include <iostream>
#include <open62541client.h>
using namespace std;
#define DISCOVERY_SERVER_ENDPOINT "opc.tcp://localhost:4850"

/*
 * This demonstrates how to access historical values. The node must have been configured as a historizing node on the server
 * Clients cannot create historizing nodes directly
 */




// read a historical node
/*!
 * \brief The HistoricalClient class
 */
class EventClient : public Open62541::Client
{
public:
    EventClient() {}

};


int main(int /*argc*/, char **/*argv*/) {
    cout << "Test Event Client - requires the TestEventServer running" << endl;
    //
    // Construct client
    EventClient client;
    // Connect
    if (client.connect("opc.tcp://localhost:4840")) {
        //
        cout << "Connected" << endl;
        Open62541::NodeId nodeNumber(2, "Number_Value"); // this is the node we want to monitor

        // loop
        // The server updates the Number_Value node every 2 seconds so if we wait 10 seconds between calls we should get 5 values
        // when we query the history
        for(;;)
        {
            //

            sleep(10);
        }
    }
    else {
        cout << "Failed to connect" << endl;
    }
    return 0;
}
