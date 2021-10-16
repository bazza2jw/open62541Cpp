#ifndef SIMULATORSTOPMETHOD_H
#define SIMULATORSTOPMETHOD_H

#include <open62541cpp/servermethod.h>
class SimulateProcess;
class SimulatorStopMethod : public Open62541::ServerMethod
{
    Open62541::Argument inputArgument1;  // argument definitions must persist
    Open62541::Argument outputArguments;
    SimulateProcess& _process;

public:
    SimulatorStopMethod(SimulateProcess& p)
        : Open62541::ServerMethod("Stop", 0, 0)
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

#endif  // SIMULATORSTOPMETHOD_H
