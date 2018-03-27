#include <iostream>
#include <open62541client.h>
using namespace std;

int main(int argc, char *argv[])
{
    cout << "Test Client" << endl;
    //
    // Construct client
    Open62541::Client client;
    // Connect
    if (client.connect("opc.tcp://localhost:4840")) {
            int idx = client.namespaceGetIndex("urn:test:test");
            cout << "Create Path in Objects" << endl;
            Open62541::Path path = {"ClientDataFolder", "UnitA"};
            Open62541::NodeId unitAFolder;
            if (client.createFolderPath(Open62541::NodeId::Objects, path, 1, unitAFolder.notNull())) {
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
                Open62541::NodeId MethodId(idx,12345);
                //
                Open62541::Variant arg0(1.25);
                Open62541::Variant arg1(3.8);
                in.push_back(arg0.get());
                in.push_back(arg1.get());

                //
                Open62541::NodeId OwnerNode(idx,"ServerMethodItem");
                if(client.callMethod(OwnerNode,MethodId,in,out))
                {
                    if(out.size() > 0)
                    {
                        UA_Double *r = (UA_Double *)(out.data()[0].data);
                        cout << "Result = " << *r << endl;
                    }
                }
                else
                {
                    UAPRINTLASTERROR(  client.lastError() );
                }
                //
                //  Subscription
                //cout << "Adding Subscription" << endl;
                //
                //
            }
            else {
                cout << "Failed to create folders" << endl;
            }
    }
    else {
        cout << "Failed to connect" << endl;
    }
    return 0;
}
