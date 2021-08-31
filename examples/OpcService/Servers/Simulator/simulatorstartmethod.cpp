#include "simulatorstartmethod.h"
#include "simulatoropc.h"
/*!
 * \brief SimulatorStartMethod::callback
 * \return
 */
UA_StatusCode SimulatorStartMethod::callback(Open62541::Server& server,
                                             const UA_NodeId* /*objectId*/,
                                             size_t /*inputSize*/,
                                             const UA_Variant* /*input*/,
                                             size_t /*outputSize*/,
                                             UA_Variant* /*output*/)
{
    _process.start();            // stop the measuring process
    Open62541::Variant v("Ok");  // set the status to OK
    server.writeValue(_process.Status, v);
    return UA_STATUSCODE_GOOD;
}
