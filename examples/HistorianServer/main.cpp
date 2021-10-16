#include <iostream>
#include <open62541cpp/open62541server.h>
#include "testcontext.h"
#include "testmethod.h"
#include <open62541cpp/serverrepeatedcallback.h>
#include "testobject.h"
#include <open62541cpp/historydatabase.h>
using namespace std;
//
// example server with memory based historian
//
/*!
 * \brief The TestServer class
 */
class TestServer : public Open62541::Server
{
    Open62541::MemoryHistorian _historian;  // the historian
    int _idx = 2;                           // namespace index
public:
    TestServer()
    {
        // Enable server as historian - must be done before starting server
        serverConfig().historyDatabase             = _historian.database();
        serverConfig().accessHistoryDataCapability = UA_TRUE;
    }
    void initialise();  // initialise the server before it runs but after it has been configured
};

/*!
 * \brief TestServer::initialise
 */
void TestServer::initialise()
{
    cout << "initialise()" << endl;
    _idx = addNamespace("urn:test:test");  // create a name space
    //
    UA_UInt64 repeatedcallbackId = 0;
    addRepeatedTimerEvent(2000, repeatedcallbackId, [&](Open62541::Server::Timer& s) {
        Open62541::NodeId nodeNumber(_idx, "Number_Value");
        int v = std::rand() % 100;
        Open62541::Variant numberValue(v);
        cout << "RepeatedEvent called setting number value = " << v << endl;
        s.server()->writeValue(nodeNumber, numberValue);
    });
    //
    cout << "Namespace " << _idx << endl;
    ;
    // Add a node and set its context to test context
    cout << "Create Historianised Node Number_Value" << endl;
    //
    Open62541::NodeId nodeNumber(_idx, "Number_Value");
    Open62541::Variant numberValue(1);
    //
    if (!addHistoricalVariable(Open62541::NodeId::Objects,
                               "Number_Value",
                               numberValue,
                               nodeNumber,
                               Open62541::NodeId::Null)) {
        cout << "Failed to create Number Value Node " << endl;
    }
    else {
        _historian.setUpdateNode(nodeNumber,
                                 *this);  // adds the node the the historian - values are buffered as they are updated
    }
    //
}

/*!
 * \brief main
 * \return
 */
int main(int /* argc*/, char** /*argv[]*/)
{
    TestServer server;
    cerr << "Starting server" << endl;
    server.start();
    cerr << "Server Finished" << endl;
    return 0;
}
