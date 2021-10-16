#ifndef SIMULATORSTARTMETHOD_H
#define SIMULATORSTARTMETHOD_H
#include <open62541cpp/servermethod.h>
class SimulateProcess;

class SimulatorStartMethod : public Open62541::ServerMethod
{
    Open62541::Argument inputArgument1;  // argument definitions must persist
    Open62541::Argument outputArguments;
    SimulateProcess& _process;

public:
    SimulatorStartMethod(SimulateProcess& p)
        : Open62541::ServerMethod("Start", 0, 0)
        , _process(p)
    {
    }

    virtual UA_StatusCode callback(Open62541::Server& /*server*/,
                                   const UA_NodeId* /*objectId*/,
                                   size_t /*inputSize*/,
                                   const UA_Variant* /*input*/,
                                   size_t /*outputSize*/,
                                   UA_Variant* /*output*/);
};

#endif  // SIMULATORSTARTMETHOD_H
