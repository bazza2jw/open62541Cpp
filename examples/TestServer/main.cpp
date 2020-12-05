#include <iostream>
#include <open62541server.h>
#include "testcontext.h"
#include "testmethod.h"
#include <serverrepeatedcallback.h>
#include <servertimedcallback.h>
#include "testobject.h"
using namespace std;
//
// example server - this exercises timers as well
//
class TestServer : public Open62541::Server {
    int _idx; // namespace index
    Open62541::ServerRepeatedCallback _repeatedEvent;
    Open62541::ServerTimedCallback _timedEvent;
    TestMethod _method;
    TestContext _context;
    TestObject _object; // object
public:
    TestServer() : _repeatedEvent(*this, 2000, [&](Open62541::ServerRepeatedCallback &s) {
        Open62541::NodeId nodeNumber(_idx, "Number_Value");
        int v = std::rand() % 100;
        Open62541::Variant numberValue(v);
        cout << "_repeatedEvent called setting number value = " << v <<  endl;
        s.server().writeValue(nodeNumber,numberValue);
    }),
    // Trigger timed event in 60 seconds
    _timedEvent(*this,[&](Open62541::ServerTimedCallback &/*s*/) {
        cout << "Timed Event Triggered " << time(0) << endl ;
    }),
    _object(*this)

    {
        cout << "Timed Event Triggers in 60 seconds Now = :" << time(0) << endl;
    }

    void initialise(); // initialise the server before it runs but after it has been configured
};

void TestServer::initialise() {
    _idx = addNamespace("urn:test:test"); // create a name space
    // Add a node and set its context to test context
    Open62541::NodeId newFolder(_idx,"ServerMethodItem");
    if (addFolder(Open62541::NodeId::Objects, "ServerMethodItem", newFolder,Open62541::NodeId::Null)) {
        // Add a string value to the folder
        Open62541::NodeId variable(_idx, "String_Value");
        Open62541::Variant v("A String Value");
        if (!addVariable(newFolder, "String_Value", v, variable, Open62541::NodeId::Null, &_context)) {
            cout << "Failed to add node " << Open62541::toString(variable)
                 << " " <<  UA_StatusCode_name(lastError()) << endl;
        }
        else {
            // attach value callbacks to this node
            if (!_context.setValueCallback(*this, variable)) {
                cout << "Failed to set value callback" << endl;
            }
        }

        cout << "Create Number_Value" << endl;
        Open62541::NodeId nodeNumber(_idx, "Number_Value");
        Open62541::Variant numberValue(1);
        if (!addVariable(Open62541::NodeId::Objects, "Number_Value", numberValue, nodeNumber, Open62541::NodeId::Null))
        {
            cout << "Failed to create Number Value Node " << endl;
        }

        //
        // Start repeated event
        //
        _repeatedEvent.start();
        _timedEvent.addSeconds(5); // trigger one shot in 5 seconds time
        _timedEvent.start();
        //
        // Create TestMethod node
        //
        Open62541::NodeId methodId(_idx, 12345);
        if (_method.addServerMethod(*this, "TestMethod", newFolder, methodId, Open62541::NodeId::Null, _idx)) {
            cout << "Added TestMethod - Adds two numbers together - call from client (e.g. UAExpert)" << endl;
        }
        else {
            cout << "Failed to add method " << " " <<  UA_StatusCode_name(lastError()) << endl;
        }
        //
        // Define an object type
        //
        Open62541::NodeId testType(_idx,"TestObjectType");
        if(_object.addType(testType))
        {
            cout << "Added TestObject type" << endl;
        }
        else
        {
            cout << "Failed to create object type" << endl;
        }

        Open62541::NodeId exampleInstance(_idx,"ExampleInstance");
        _object.addInstance("ExampleInstance",newFolder,exampleInstance);
    }
    else {
        cout << "Failed to add folder " << " " <<  UA_StatusCode_name(lastError()) << endl;
    }

}

int main(int/* argc*/, char **/*argv[]*/) {
    TestServer server;
    cerr << "Starting server" << endl;
    server.start();
    cerr << "Server Finished" << endl;
    return 0;
}
