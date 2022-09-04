#include <iostream>
#include <open62541cpp/open62541client.h>
#include <signal.h>
#include <stdlib.h>
using namespace std;
#define DISCOVERY_SERVER_ENDPOINT "opc.tcp://localhost:4850"

int main(int /*argc*/, char** /*argv*/)
{
    cout << "Test Client" << endl;
    //
    // Construct client
    Open62541::Client client;
    // Connect
    try {
        client.connect("opc.tcp://localhost:4840");
    }
    catch (const Open62541::Exception& e) {
        cout << "Failed to connect: " << e.what() << std::endl;
        return 1;
    }
    //

    int idx = client.namespaceGetIndex("urn:test:test");
    cout << "Get Endpoints" << endl;
    Open62541::EndpointDescriptionArray ea;
    client.getEndpoints("opc.tcp://localhost:4840", ea);
    //
    for (size_t i = 0; i < ea.length(); i++) {
        cout << "End Point " << i << " = " << Open62541::toString(ea.at(i).endpointUrl) << endl;
    }
    //
    // Browse for servers

    //
    cout << "Create Path in Objects" << endl;
    //
    Open62541::Path path = {"ClientDataFolder", "UnitA"};
    Open62541::NodeId unitAFolder;

    try {
        client.createFolderPath(Open62541::NodeId::Objects, path, 1, unitAFolder.notNull());
    }
    catch (const Open62541::Exception& e) {
        cout << "Failed to create folders: " << e.what() << std::endl;
        return 1;
    }

    cout << "Create Variable on Server" << endl;
    //
    Open62541::NodeId variable(1, "A_Value");
    Open62541::Variant v(double(98.76));
    Open62541::NodeId newVariable;
    client.addVariable(unitAFolder, "A_Value", v, variable, newVariable.notNull());
    // Call Hello method
    cout << "Call TestHello method in server" << endl;
    Open62541::VariantList in;
    Open62541::VariantCallResult out;
    Open62541::NodeId MethodId(idx, 12345);
    //
    Open62541::Variant arg0(1.25);
    Open62541::Variant arg1(3.8);
    in.push_back(arg0.get());
    in.push_back(arg1.get());

    //
    Open62541::NodeId OwnerNode(idx, "ServerMethodItem");
    try {
        client.callMethod(OwnerNode, MethodId, in, out);
        if (out.size() > 0) {
            UA_Double* r = (UA_Double*)(out.data()[0].data);
            cout << "Result = " << *r << endl;
        }
    }
    catch (const Open62541::Exception& e) {
        cout << "Failed to call method: " << e.what() << std::endl;
        return 1;
    }

    //
    // Discover servers
    cout << "Discovery of Servers" << endl;
    //
    Open62541::StringArray serverUris;
    Open62541::StringArray localeIds;
    Open62541::ApplicationDescriptionArray registeredServers;
    Open62541::Client discoveryClient;
    discoveryClient.initialise();
    //
    try {
        discoveryClient.findServers(DISCOVERY_SERVER_ENDPOINT, serverUris, localeIds, registeredServers);
        cout << "Discovered Number of Servers: " << registeredServers.length() << endl;
        for (size_t i = 0; i < registeredServers.length(); i++) {

            UA_ApplicationDescription& description = registeredServers.at(i);
            cout << "Server [" << i << "]: " << description.applicationUri.length << description.applicationUri.data
                 << endl;
            cout << "\n\tName [" << description.applicationName.text.length
                 << "] : " << description.applicationName.text.data << endl;
            cout << "\n\tApplication URI: " << description.applicationUri.length << description.applicationUri.data
                 << endl;
            cout << "\n\tProduct URI: " << description.productUri.length << " " << description.productUri.data << endl;
            cout << "\n\tType: ";
            switch (description.applicationType) {
                case UA_APPLICATIONTYPE_SERVER:
                    cout << "Server";
                    break;
                case UA_APPLICATIONTYPE_CLIENT:
                    cout << "Client";
                    break;
                case UA_APPLICATIONTYPE_CLIENTANDSERVER:
                    cout << "Client and Server";
                    break;
                case UA_APPLICATIONTYPE_DISCOVERYSERVER:
                    cout << "Discovery Server";
                    break;
                default:
                    cout << "Unknown";
            }
            cout << endl << "\tDiscovery URLs:";
            for (size_t j = 0; j < description.discoveryUrlsSize; j++) {
                cout << endl
                     << "\t\t" << j << " " << description.discoveryUrls[j].length << " "
                     << description.discoveryUrls[j].data << endl;
            }
            cout << endl;
        }
    }
    catch (const Open62541::Exception& e) {
        cout << "Failed to call method: " << e.what() << std::endl;
        return 1;
    }

    cout << "Test Timers" << endl;

    // Now run the timer tests
    // set up timed event for 10 seconds time
    UA_UInt64 callerId;
    client.addTimedEvent(10000, callerId, [](Open62541::Client::Timer&) {
        std::cerr << "Timed Event Triggered " << time(0) << std::endl;
    });
    std::cerr << "Added one shot timer event for 10 seconds time  Now = " << time(0) << " Id = " << callerId << endl;
    //
    // Add a repeated timer event - these can be thought of as event driven tasks
    //
    client.addRepeatedTimerEvent(2000, callerId, [](Open62541::Client::Timer&) {
        std::cerr << "Repeated Event Triggered " << time(0) << std::endl;
    });
    std::cerr << "Added repeated timer event for 2 seconds  Now = " << time(0) << " Id = " << callerId << endl;
    //
    client.run();  // this will loop until interrupted
}
