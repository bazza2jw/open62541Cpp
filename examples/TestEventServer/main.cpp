#include <iostream>
#include <open62541cpp/open62541server.h>
#include "testmethod.h"

using namespace std;
//
// Events - server side
//
/*!
    \brief The TestServer class
*/
class TestServer : public Open62541::Server
{
    int _idx = 2;  // namespace index
    TestMethod _method;
    Open62541::NodeId eventType;

public:
    TestServer() {}
    void initialise();  // initialise the server before it runs but after it has been configured
};

/*!
    \brief TestServer::initialise
*/
void TestServer::initialise()
{
    cout << "TestEventServer - call the TestEventTriggerMethod from UA Expert (for example) to trigger events " << endl;
    _idx = addNamespace("urn:test:test");  // create a name space
    //
    cout << "Namespace " << _idx << endl;
    _method.initialise(*this);
    //
    // Add a node and set its context to test context
    //
    Open62541::NodeId newFolder(_idx, "ServerMethodItem");
    //
    addFolder(Open62541::NodeId::Objects, "ServerMethodItem", newFolder, Open62541::NodeId::Null);
    //
    Open62541::NodeId nodeNumber(_idx, "Number_Value");
    Open62541::Variant numberValue(1);
    //
    addVariable(Open62541::NodeId::Objects, "Number_Value", numberValue, nodeNumber, Open62541::NodeId::Null);

    Open62541::NodeId methodId(_idx, "EventTrigger");
    _method.addServerMethod(*this, "TestEventTriggerMethod", newFolder, methodId, Open62541::NodeId::Null, _idx);
}

/*!
    \brief main
    \return
*/
int main(int /* argc*/, char** /*argv[]*/)
{
    TestServer server;
    cerr << "Starting server" << endl;
    server.start();
    cerr << "Server Finished" << endl;
    return 0;
}
