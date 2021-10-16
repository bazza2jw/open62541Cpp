#include <iostream>
#define UA_TRACE_OBJ
#include <open62541cpp/open62541objects.h>
using namespace std;

int main(int /*argc*/, char** /*argv[]*/)
{
    {
        cout << "Variable Test" << endl;
        Open62541::NodeId A(1, "Node A");
        Open62541::NodeId B(1, "Node B");
        Open62541::NodeId C(1, "Node C");
        cout << "At Start A = " << Open62541::toString(A) << " B = " << Open62541::toString(B) << " C "
             << Open62541::toString(C) << endl;
        cout << "Assign A to C" << endl;
        C = A;
        cout << "After Assign A = " << Open62541::toString(A) << " B = " << Open62541::toString(B) << " C "
             << Open62541::toString(C) << endl;
        //
        //
        cout << "Assigning C types Test" << endl;
        UA_NodeId x = UA_NODEID_NUMERIC(1, 1234);  // these should be explicity deleted using UA_NodeId_clear
        UA_NodeId y = UA_NODEID_NUMERIC(1, 4567);
        UA_NodeId z = UA_NODEID_NUMERIC(1, 9876);
        //
        Open62541::NodeId D(x);  // take copy and own
        Open62541::NodeId E = y;
        Open62541::NodeId F(z);
        cout << " D = " << Open62541::toString(D) << " E = " << Open62541::toString(E) << " F "
             << Open62541::toString(F) << endl;

        cout << "Expect Final Delete of Z" << endl;
        F = D;
        cout << "Report D,E,F" << endl;
        //
        cout << " D = " << Open62541::toString(D) << " E = " << Open62541::toString(E) << " F "
             << Open62541::toString(F) << endl;

        cout << "End of scope" << endl;
    }
    cout << "Exited test scope" << endl;
    return 0;
}
