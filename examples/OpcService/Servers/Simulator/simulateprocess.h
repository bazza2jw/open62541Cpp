#ifndef SIMULATEPROCESS_H
#define SIMULATEPROCESS_H
#include <open62541cpp/open62541server.h>
#include <open62541cpp/serverrepeatedcallback.h>
#include <OpcServiceCommon/opcservicecommon.h>
#include "simulatordefs.h"
#include <OpcServiceCommon/stockdefs.h>
#include "simulatornodecontext.h"
#include "simulatorstartmethod.h"
#include "simulatorstopmethod.h"

enum { ValueId = 1000, StatusId, RangeId, TypeId, IntervalId };

/*!
 * \brief The SimulateProcess class
 * This is a data collection process driven on a timer
 */
class SimulateProcess : public Open62541::SeverRepeatedCallback
{
    int _ticks     = 0;
    int _lastValue = 0;     // the last generated value
    bool _dirUp    = true;  // ramp direction
    int _idx;               // The namespace
    //
    SimulatorNodeContext _context;
    SimulatorStartMethod _startMethod;
    SimulatorStopMethod _stopMethod;
    //
    // The node ids used
    //
public:
    // Node references
    Open62541::NodeId Value;
    Open62541::NodeId Status;
    Open62541::NodeId Range;
    Open62541::NodeId Type;
    Open62541::NodeId Interval;
    //
public:
    /*!
        \brief SimulateProcess
        \param s
    */
    SimulateProcess(Open62541::Server& s, int ns = 2);
    /*!
        \brief callback
    */
    void callback();
};

#endif  // SIMULATEPROCESS_H
