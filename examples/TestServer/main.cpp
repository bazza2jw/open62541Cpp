#include <iostream>
#include <open62541server.h>
#include "testcontext.h"
#include "testmethod.h"
#include "testobject.h"
using namespace std;
//
// example server - this exercises timers as well
class EventContext : public Open62541::NodeContext
{
public:
    EventContext() : Open62541::NodeContext("Event") {}

};

//
class TestServer : public Open62541::Server {
    int _idx; // namespace index
    TestMethod _method;
    TestContext _context;
    TestObject _object; // object
    Open62541::NodeId _testTriggerSource;
    Open62541::ServerMethod _eventMethod; // no arguments uses functor to handle
    Open62541::NodeId _eventType;
    Open62541::NodeId _eventNode;
public:
    TestServer() :  _object(*this),_eventMethod("EventTest",[this](Open62541::Server &server,const UA_NodeId *, size_t, const UA_Variant *, size_t,UA_Variant *) {
        std::cerr << "Event Trigger Called " << std::endl;
        UA_ByteString bs;
        createEvent(_eventType,_eventNode);
        server.triggerEvent(_eventNode,_testTriggerSource,&bs,false);
        return UA_StatusCode(UA_STATUSCODE_GOOD);
    })
    {
        addNewEventType("SimpleEventType",_eventType,"The simple event type we created");
        _eventNode.notNull();
        setUpEvent(_eventNode,_eventType,"SimleEvent","TestServer");
    }
    void initialise(); // initialise the server before it runs but after it has been configured
};

void TestServer::initialise() {
    _idx = addNamespace("urn:test:test"); // create a name space

    // Add the timers
    UA_UInt64 repeatedcallbackId = 0;
    addRepeatedTimerEvent(2000, repeatedcallbackId, [&](Open62541::Server::Timer &s) {
        Open62541::NodeId nodeNumber(_idx, "Number_Value");
        int v = std::rand() % 100;
        Open62541::Variant numberValue(v);
        cout << "_repeatedEvent called setting number value = " << v <<  endl;
        s.server()->writeValue(nodeNumber,numberValue);
    });

    // Add one shot timer
    UA_UInt64 timedCallback = 0;
    addTimedEvent(5000,timedCallback,[&](Open62541::Server::Timer &/*s*/) {
        cout << "Timed Event Triggered " << time(0) << endl ;
    });

    // Add a node and set its context to test context
    Open62541::NodeId newFolder(_idx,"ServerMethodItem");
    if (addFolder(Open62541::NodeId::Objects, "ServerMethodItem", newFolder,Open62541::NodeId::Null)) {
        // Add a string value to the folder
        Open62541::NodeId variable(_idx, "String_Value");
        Open62541::Variant v("A String Value");
        if (!addVariable(Open62541::NodeId::Objects, "String_Value", v, variable, Open62541::NodeId::Null, &_context)) {
            cout << "Failed to add node " << Open62541::toString(variable)
                 << " " <<  UA_StatusCode_name(lastError()) << endl;
        }
        else {
            // attach value callbacks to this node
            if (!_context.setValueCallback(*this, variable)) {
                cout << "Failed to set value callback" << endl;
            }
        }

        // Set up an event source - monitor this item to get the events in UA Expert
        _testTriggerSource.notNull();
        if (!addVariable(Open62541::NodeId::Objects, "TestTrigger", v, Open62541::NodeId::Null, _testTriggerSource)) {
            cout << "Failed to add node " << Open62541::toString(variable)
                 << " " <<  UA_StatusCode_name(lastError()) << endl;
        }


        cout << "Create Number_Value" << endl;
        Open62541::NodeId nodeNumber(_idx, "Number_Value");
        Open62541::Variant numberValue(1);
        if (!addVariable(Open62541::NodeId::Objects, "Number_Value", numberValue, nodeNumber, Open62541::NodeId::Null))
        {
            cout << "Failed to create Number Value Node " << endl;
        }
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
        //
        // Add the event method
        //
        Open62541::NodeId eventMethodId(_idx, 12346);
        if (_eventMethod.addServerMethod(*this, "EventMethod", newFolder, eventMethodId, Open62541::NodeId::Null, _idx)) {
            cout << "Added EventMethod" << endl;
        }
        else {
            cout << "Failed to add method " << " " <<  UA_StatusCode_name(lastError()) << endl;
        }


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
