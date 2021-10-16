#include <open62541cpp/open62541server.h>
#include <open62541cpp/open62541client.h>
#include <open62541cpp/serverrepeatedcallback.h>
/*!
    This is an example server that registers with the discover y server
*/

using namespace std;
#define DISCOVERY_SERVER_ENDPOINT "opc.tcp://localhost:4850"

// example discoverable server - give port and server name as arguments
/*!
 * \brief The TestServer class
 */
class TestServer : public Open62541::Server
{
    int _idx;  // namespace index
    //
    Open62541::Client _client;

public:
    TestServer(int port)
        : Open62541::Server(port)
    {
    }

    void initialise();  // initialise the server before it runs but after it has been configured
};
/*!
 * \brief TestServer::initialise
 */
void TestServer::initialise()
{
    _idx = addNamespace("urn:test:test");  // create a name space

    // Add the timers
    UA_UInt64 repeatedcallbackId = 0;
    addRepeatedTimerEvent(2000, repeatedcallbackId, [&](Open62541::Server::Timer& s) {
        Open62541::NodeId nodeNumber(_idx, "Number_Value");
        int v = std::rand() % 100;
        Open62541::Variant numberValue(v);
        cout << "_repeatedEvent called setting number value = " << v << endl;
        s.server()->writeValue(nodeNumber, numberValue);
    });

    // Add one shot timer
    UA_UInt64 timedCallback = 0;
    addTimedEvent(5000, timedCallback, [&](Open62541::Server::Timer& /*s*/) {
        cout << "Timed Event Triggered " << time(0) << endl;
    });

    // Add a node and set its context to test context
    Open62541::NodeId newFolder(_idx, "ServerItems");
    if (addFolder(Open62541::NodeId::Objects, "ServerItems", newFolder, Open62541::NodeId::Null)) {
        cout << "Create Number_Value" << endl;
        Open62541::NodeId nodeNumber(_idx, "Number_Value");
        Open62541::Variant numberValue(1);
        if (!addVariable(Open62541::NodeId::Objects,
                         "Number_Value",
                         numberValue,
                         nodeNumber,
                         Open62541::NodeId::Null)) {
            cout << "Failed to create Number Value Node " << endl;
        }
        //
        // Start repeated event - so it does something
        // connect to the discovery server
        if (_client.connect(DISCOVERY_SERVER_ENDPOINT)) {
            cerr << "Register with discovery server" << endl;
            if (!registerDiscovery(_client)) {
                cerr << "Failed to register with discovery server" << endl;
            }
            else {
                cerr << "Registered with discovery server" << endl;
            }
        }
        else {
            cerr << "Failed to connect with discovery server" << endl;
        }
    }
}
/*!
 * \brief main
 * \param argc
 * \param argv
 * \return
 */
int main(int argc, char** argv)
{
    if (argc != 3) {
        cerr << "Usage: ServerDiscoverable <port> <Server Name>" << endl;
    }
    else {
        //
        int port = ::atoi(argv[1]);
        std::string name(argv[2]);
        //
        cerr << "Port " << port << " Name " << name << endl;
        TestServer server(port);
        server.setMdnsServerName(name);
        server.setServerUri("Test Discoverable Server");
        cerr << "Starting server" << endl;
        server.start();
        cerr << "Server Finished" << endl;
    }
    return 0;
}
