#include "simulatorstopmethod.h"
#include "simulatoropc.h"
/*!
 * \brief SimulatorStopMethod::callback
 * \param server
 * \return UA_STATUSCODE_GOOD
 */
UA_StatusCode SimulatorStopMethod::callback(Open62541::Server& server,
                                            const UA_NodeId* /*objectId*/,
                                            size_t /*inputSize*/,
                                            const UA_Variant* /*input*/,
                                            size_t /*outputSize*/,
                                            UA_Variant* /*output*/)
{
    _process.stop();                  // stop the measuring process
    Open62541::Variant v("Stopped");  // set the status to stopped
    server.writeValue(_process.Status, v);
    return UA_STATUSCODE_GOOD;
}
