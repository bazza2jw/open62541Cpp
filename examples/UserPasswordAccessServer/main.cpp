#include <iostream>
#include <open62541cpp/open62541server.h>
using namespace std;
//
// example server with memory based historian
//
/*!
    \brief The TestServer class
*/
class TestServer : public Open62541::Server
{
    int _idx = 2;  // namespace index

public:
    TestServer()
    {
        logins().resize(1);
        logins()[0].username = UA_STRING_STATIC("admin");
        logins()[0].password = UA_STRING_STATIC("password");
        enableSimpleLogin();
    }
    void initialise();  // initialise the server before it runs but after it has been configured
};

/*!
    \brief TestServer::initialise
*/
void TestServer::initialise()
{
    cout << "initialise()" << endl;
    _idx = addNamespace("urn:test:test");  // create a name space
    //
    Open62541::NodeId nodeNumber(_idx, "Number_Value");
    Open62541::Variant numberValue(1);
    //
    if (!addVariable(Open62541::NodeId::Objects, "Number_Value", numberValue, nodeNumber, Open62541::NodeId::Null)) {
        cout << "Failed to create Number Value Node " << endl;
    }
    //
    // Set the user name and password
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
