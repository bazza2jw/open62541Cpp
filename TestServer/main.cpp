#include <iostream>
#include <open62541server.h>
#include "testcontext.h"
#include "testmethod.h"
#include <serverrepeatedcallback.h>
#include "testobject.h"
using namespace std;

// example server
class TestServer : public Open62541::Server {
        int _idx; // namespace index
        Open62541::SeverRepeatedCallback _repeatedEvent; //
        TestMethod _method;
        TestContext _context;
        TestObject _object; // object
    public:
        TestServer() : _repeatedEvent(*this, 2000, [](Open62541::SeverRepeatedCallback &) {
            cout << "_repeatedEvent called" << endl;
        }),
        _object(*this)

        {

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
        //
        // Start repeated event
        //
        _repeatedEvent.start();
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
