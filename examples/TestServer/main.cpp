#include <iostream>
#include <open62541cpp/open62541server.h>
#include "testcontext.h"
#include "testmethod.h"
#include "testobject.h"
using namespace std;
//
// example server - this exercises timers as well
class EventContext : public Open62541::NodeContext
{
public:
    EventContext()
        : Open62541::NodeContext("Event")
    {
    }
};

//
class TestServer : public Open62541::Server
{
    int _idx{};  // namespace index
    TestMethod _method;
    TestContext _context;
    TestObject _object;  // object
    Open62541::NodeId _testTriggerSource;
    Open62541::ServerMethod _eventMethod;  // no arguments uses functor to handle
    Open62541::NodeId _eventType;
    Open62541::NodeId _eventNode;

public:
    TestServer(int port = 4840) : Open62541::Server(port),
         _object(*this)
        , _eventMethod(
              "EventTest",
              [this](Open62541::Server& server, const UA_NodeId*, size_t, const UA_Variant*, size_t, UA_Variant*) {
                  std::cerr << "Event Trigger Called " << std::endl;
                  UA_ByteString bs;
                  createEvent(_eventType, _eventNode);
                  server.triggerEvent(_eventNode, _testTriggerSource, &bs, false);
                  return UA_StatusCode(UA_STATUSCODE_GOOD);
              })
    {
        addNewEventType("SimpleEventType", _eventType, "The simple event type we created");
        _eventNode.notNull();
        setUpEvent(_eventNode, _eventType, "SimpleEvent", "TestServer");
    }
    void initialise() override;  // initialise the server before it runs but after it has been configured
};

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
    Open62541::NodeId newFolder(_idx, "ServerMethodItem");
    if (!addFolder(Open62541::NodeId::Objects, "ServerMethodItem", newFolder, Open62541::NodeId::Null)) {
        cout << "Failed to add folder "
             << " " << UA_StatusCode_name(lastError()) << endl;
        return;
    }

    // Add a string value to the folder
    Open62541::NodeId variable(_idx, "String_Value");
    Open62541::Variant v("A String Value");
    if (!addVariable(Open62541::NodeId::Objects, "String_Value", v, variable, Open62541::NodeId::Null, &_context)) {
        cout << "Failed to add node " << Open62541::toString(variable) << " " << UA_StatusCode_name(lastError())
             << endl;
    }
    else {
        // attach value callbacks to this node
        if (!_context.setValueCallback(*this, variable)) {
            cout << "Failed to set value callback" << endl;
        }
    }
    
   //Example adding an array by setting it with setArrayCopy
    Open62541::NodeId setArrayCopy_array_id(_idx, "Array_By_Copy");
    UA_Double setArrayCopy_temp_array[4] = {1.0, 2.0, 3.0, 4.0};
    const size_t array_size = 4;
    Open62541::Variant setArrayCopy_variant;
    setArrayCopy_variant.setArrayCopy(&setArrayCopy_temp_array, array_size, &UA_TYPES[UA_TYPES_DOUBLE]);
    addVariable(Open62541::NodeId::Objects, "Array_By_Copy", setArrayCopy_variant , setArrayCopy_array_id, Open62541::NodeId::Null);

    //Example adding an array by setting it with setArray as a child for Array_With_Copy
    Open62541::NodeId setArray_array_id(_idx, "Array_By_Set");
    Open62541::Variant setArray_variant;
    const size_t setArray_array_size = 3;
    UA_Double *setArray_array = (UA_Double *) UA_Array_new(3, &UA_TYPES[UA_TYPES_DOUBLE]);
    setArray_array[0] = 1;
    setArray_array[1] = 3;
    setArray_array[2] = 5;
    setArray_variant.setArray(setArray_array , setArray_array_size, &UA_TYPES[UA_TYPES_DOUBLE]);
    addVariable(setArrayCopy_array_id, "Array_By_Set", setArray_variant , setArray_array_id, Open62541::NodeId::Null);

    //Example adding a matrix 
    Open62541::NodeId test_matrix_id(_idx, "Matrix_Example");
    Open62541::VariableAttributes vattr;
    Open62541::Variant vattr_value;
    //set required dimensions and values for the matrix
    UA_Double matrix_temp_array[6] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    UA_Int32 rows = 3;
    UA_Int32 cols = 2;
    size_t matrix_dims = 2;
    int value_rank = 2;
    vattr_value = vattr.getVariantMatrix(rows, cols, matrix_dims, &UA_TYPES[UA_TYPES_DOUBLE], value_rank, &matrix_temp_array);
    addVariable(Open62541::NodeId::Objects, "Matrix_Example", vattr_value, test_matrix_id, Open62541::NodeId::Null);

    // Set up an event source - monitor this item to get the events in UA Expert
    _testTriggerSource.notNull();
    if (!addVariable(Open62541::NodeId::Objects, "TestTrigger", v, Open62541::NodeId::Null, _testTriggerSource)) {
        cout << "Failed to add node " << Open62541::toString(variable) << " " << UA_StatusCode_name(lastError())
             << endl;
    }

    cout << "Create Number_Value" << endl;
    Open62541::NodeId nodeNumber(_idx, "Number_Value");
    Open62541::Variant numberValue(1);
    if (!addVariable(Open62541::NodeId::Objects, "Number_Value", numberValue, nodeNumber, Open62541::NodeId::Null)) {
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
        cout << "Failed to add method "
             << " " << UA_StatusCode_name(lastError()) << endl;
    }
    //
    // Define an object type
    //
    Open62541::NodeId testType(_idx, "AR_ObjectType");
    if (!_object.addType(testType)) {
        cout << "Failed to create object type" << endl;
    }
    else {
        cout << "Added TestObject type" << endl;
    }

    Open62541::NodeId exampleInstance(_idx, "ExampleInstance");
    _object.addInstance("ExampleInstance", newFolder, exampleInstance);
    //
    // Add the event method
    //
    Open62541::NodeId eventMethodId(_idx, 12346);
    if (_eventMethod.addServerMethod(*this, "EventMethod", newFolder, eventMethodId, Open62541::NodeId::Null, _idx)) {
        cout << "Added EventMethod" << endl;
    }
    else {
        cout << "Failed to add method "
             << " " << UA_StatusCode_name(lastError()) << endl;
    }
}

TestServer* server_instance = nullptr;
inline void StopHandler(int /*unused*/)
{
    if (server_instance)
        server_instance->stop();
    std::cout << "preparing to shut down..."
              << "\n";
}

inline void SetupSignalHandlers()
{
    signal(SIGINT, StopHandler);
    signal(SIGTERM, StopHandler);
}

int main(int  argc, char** argv)
{
    int port = 4840;
    SetupSignalHandlers();
    if(argc > 1) port = std::atoi(argv[1]);
    TestServer server(port);
    server_instance = &server;
    cerr << "Starting server" << endl;
    server.start();
    cerr << "Server Finished" << endl;
    return 0;
}
