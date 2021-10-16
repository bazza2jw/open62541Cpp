#include <iostream>
#include <open62541cpp/discoveryserver.h>
using namespace std;
#define DISCOVERY_SERVER_ENDPOINT "opc.tcp://localhost:4850"
int main()
{
    cout << "Discovery Server " DISCOVERY_SERVER_ENDPOINT << endl;
    Open62541::DiscoveryServer server(4850, DISCOVERY_SERVER_ENDPOINT);
    server.run();
    cout << "Discovery Server Exit" << endl;
    return 0;
}
