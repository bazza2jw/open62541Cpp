#ifndef TESTMETHOD_H
#define TESTMETHOD_H

#include <open62541cpp/servermethod.h>
class TestMethod : public Open62541::ServerMethod
{
    Open62541::Argument inputArgument1; // argument definitions must persist because shallow copies used of UA_Argument
    Open62541::Argument inputArgument2; //
    Open62541::Argument outputArguments1;
    Open62541::Argument outputArguments2;

public:
    TestMethod() : Open62541::ServerMethod("AddNumbers",2,2)
    {
        in()[0] = inputArgument1.set(UA_TYPES_DOUBLE,"Argument 1","Argument 1");
        in()[1] = inputArgument2.set(UA_TYPES_DOUBLE,"Argument 2","Argument 2");
        out()[0] = outputArguments1.set(UA_TYPES_DOUBLE,"Sum","Addition of Numbers");
        out()[1] = outputArguments2.set(UA_TYPES_STRING,"Output string","test a sting");

        setFunction( [](Open62541::Server &/*server*/,const UA_NodeId */*objectId*/, size_t inputSize, const UA_Variant *input, size_t outputSize,UA_Variant *output) {
            
            // This method adds two numbers and returns the result
            if(inputSize == 2 && outputSize == 2) // validate argument lists are the correct length
            {
                UA_Double *arg1 = (UA_Double *)input[0].data; // assume double - but should validate
                UA_Double *arg2 = (UA_Double *)input[1].data;
                double sum = *arg1 + *arg2;
                Open62541::String test_string("This is a 2nd Test Output argument of type String");
                UA_Variant_setScalarCopy(&output[0], &sum, &UA_TYPES[UA_TYPES_DOUBLE]);
                UA_Variant_setScalarCopy(&output[1], &test_string, &UA_TYPES[UA_TYPES_STRING]);
            }
            return UA_StatusCode(UA_STATUSCODE_GOOD);
        });
    }

};

#endif // TESTMETHOD_H
