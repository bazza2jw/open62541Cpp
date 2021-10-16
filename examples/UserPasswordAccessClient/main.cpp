#include <iostream>
#include <open62541cpp/open62541client.h>
using namespace std;

int main(int /*argc*/, char** /*argv*/)
{
    cout << "Test Client - User/Password access test" << endl;
    //
    // Construct client
    Open62541::Client client;
    // Connect with correct username password
    if (client.connectUsername("opc.tcp://localhost:4840", "admin", "password")) {
        cout << "PASS Connected" << endl;
    }
    else {
        cout << "ERROR Failed to connect " << endl;
    }
    client.disconnect();

    if (client.connectUsername("opc.tcp://localhost:4840", "Admin", "password")) {
        cout << "ERROR Connected - Invalid Username" << endl;
    }
    else {
        cout << "PASS Failed to connect - Invalid Username" << endl;
    }

    client.disconnect();

    if (client.connectUsername("opc.tcp://localhost:4840", "admin", "Password")) {
        cout << "ERROR Connected - Invalid Password" << endl;
    }
    else {
        cout << "PASS Failed to connect - Invalid Password" << endl;
    }

    client.disconnect();

    return 0;
}
