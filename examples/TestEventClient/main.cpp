#include <iostream>
#include "EventClient.h"

using namespace std;
#define DISCOVERY_SERVER_ENDPOINT "opc.tcp://localhost:4850"

// Event client side

int main(int /*argc*/, char** /*argv*/)
{
    cout << "Test Event Client - requires the TestEventServer running - use UA Expert (for example) to trigger events "
            "on the server"
         << endl;
    //
    // Construct client
    EventClient client;
    // Connect
    try {
        client.connect("opc.tcp://localhost:4840");
    }
    catch (const Open62541::Exception& e) {
        cout << "Failed to connect: " << e.what() << std::endl;
        return 1;
    }

    client.subscribe();  // subscribe
    client.run();

    return 0;
}
