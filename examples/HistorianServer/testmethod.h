#ifndef TESTMETHOD_H
#define TESTMETHOD_H

#include <open62541cpp/servermethod.h>
class TestMethod : public Open62541::ServerMethod
{
    Open62541::Argument inputArgument1;  // argument definitions must persist
    Open62541::Argument inputArgument2;  // argument definitions must persist
    Open62541::Argument outputArguments;

public:
    TestMethod()
        : Open62541::ServerMethod("AddNumbers", 2, 1)
    {
        inputArgument1.setDataType(UA_TYPES_DOUBLE);
        inputArgument1.setDescription("Argument 1");
        inputArgument1.setName("Argument 1");
        inputArgument1.setValueRank(-1); /* scalar argument */
        inputArgument2.setDataType(UA_TYPES_DOUBLE);
        inputArgument2.setDescription("Argument 2");
        inputArgument2.setName("Argument 2");
        inputArgument2.setValueRank(-1); /* scalar argument */

        in()[0] = inputArgument1.get();
        in()[1] = inputArgument2.get();

        outputArguments.setDataType(UA_TYPES_DOUBLE);
        outputArguments.setDescription("Addition of Numbers");
        outputArguments.setName("Sum");
        outputArguments.setValueRank(-1);
        out()[0] = outputArguments.get();
    }
    /*!
        \brief callback
        \return
    */
    virtual UA_StatusCode callback(Open62541::Server& /*server*/,
                                   const UA_NodeId* /*objectId*/,
                                   size_t /*inputSize*/,
                                   const UA_Variant* /*input*/,
                                   size_t /*outputSize*/,
                                   UA_Variant* /*output*/);
};

#endif  // TESTMETHOD_H
