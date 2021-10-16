#ifndef SIMULATOROPC_H
#define SIMULATOROPC_H
#include <open62541cpp/open62541server.h>
#include <open62541cpp/serverrepeatedcallback.h>
#include <OpcServiceCommon/opcservicecommon.h>
#include "simulatordefs.h"
#include <OpcServiceCommon/stockdefs.h>
#include "simulateprocess.h"

/*!
    \brief The SimulatorOpc class
    The Simulator OPC server
    This is the simpler version. The values and methods are created from the start. On simulator server has one (fixed)
   set of values. An alternative approach is to create a simulator type object (or objects)  and bind its functionality
   to the instance node context so many simulators could be created. For the simulator this would work fine but for say
   a WaveShare I/O board on a raspberry pi, which can only have one instance this would not be useful.
*/
class SimulatorOpc : public Open62541::Server
{
    //
    int _idx;  // namespace index
    UA_UInt64 _discoveryid;
    std::string _discoveryServerEndpoint;
    std::unique_ptr<SimulateProcess> _process;
    //
    static SimulatorOpc* _instance;

public:
    SimulatorOpc(int port)
        : Server(port)
    {
        _instance = this;
    }
    void initialise();  // initialise the server before it runs but after it has been configured
    static SimulatorOpc* instance() { return _instance; }
    SimulateProcess* getProcess() { return _process.get(); }
};

#endif  // SIMULATOROPC_H
