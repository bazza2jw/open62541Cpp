#include "simulatoropc.h"
#include <OpcServiceCommon/stockdefs.h>
SimulatorOpc* SimulatorOpc::_instance = nullptr;

/*!
 * \brief SimulatorOpc::initialise
 */
void SimulatorOpc::initialise()
{
    Open62541::Server::initialise();
    _idx = addNamespace("urn:simulator");  // create a name space
    Open62541::NodeId newFolder(_idx, "Simulator");
    if (addFolder(Open62541::NodeId::Objects, "Simulator", newFolder, Open62541::NodeId::Null)) {
        // periodic processing
        _process = std::make_unique<SimulateProcess>(*this, _idx);
        _process->start();
        //
        // Make this server discoverable
        // if (!addPeriodicServerRegister(DISCOVERY_SERVER_ENDPOINT, _discoveryid)) {
        //    std::cerr << "Failed to register with discovery server" << endl;
        //}
    }
}
