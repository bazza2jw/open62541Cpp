#include "simulateprocess.h"
#include <OpcServiceCommon/opcservicecommon.h>
#include <Wt/WServer>
#include "simulatorapp.h"
/*!
    \brief SimulateProcess
    \param s
*/
SimulateProcess::SimulateProcess(Open62541::Server& s, int ns)
    : Open62541::SeverRepeatedCallback(s, 1000)
    , _idx(ns)
    , _startMethod(*this)
    , _stopMethod(*this)
    , Value(ns, ValueId)
    , Status(ns, StatusId)
    , Range(ns, RangeId)
    , Type(ns, TypeId)
    , Interval(ns, IntervalId)
{
    Open62541::NodeId folder(_idx, "Simulator");
    Open62541::Variant numberValue(0);
    Open62541::Variant statusValue("Ok");
    //
    s.addVariable(folder, STOCKDEFS::Value, numberValue, Value, Open62541::NodeId::Null);
    s.addVariable(folder, STOCKDEFS::Status, statusValue, Status, Open62541::NodeId::Null);
    s.addVariable(folder, "Range", numberValue, Range, Open62541::NodeId::Null, &_context);
    s.addVariable(folder, "Type", numberValue, Type, Open62541::NodeId::Null, &_context);
    s.addVariable(folder, "Interval", numberValue, Interval, Open62541::NodeId::Null, &_context);
    //
    _context.setAsDataSource(s, Range);     // install read/write value handler
    _context.setAsDataSource(s, Type);      // install read/write value handler
    _context.setAsDataSource(s, Interval);  // install read/write value handler
    //
    Open62541::NodeId startMethodId(_idx, "Start");
    Open62541::NodeId stopMethodId(_idx, "Stop");
    //
    _startMethod.addServerMethod(s, "Start", folder, startMethodId, Open62541::NodeId::Null, _idx);
    _stopMethod.addServerMethod(s, "Stop", folder, stopMethodId, Open62541::NodeId::Null, _idx);
    //
}
/*!
    \brief callback
*/
void SimulateProcess::callback()
{
    // get the current parameters
    _ticks++;
    MRL::PropertyPath cfg;
    cfg.push_back(STOCKDEFS::ConfigureSection);
    int i = int(MRL::OpcServiceCommon::data().getValue<double>(cfg, "Interval"));
    if (i && !(_ticks % i)) {
        int r = int(MRL::OpcServiceCommon::data().getValue<double>(cfg, "Range"));
        int t = int(MRL::OpcServiceCommon::data().getValue<double>(cfg, "Type"));
        if (r < 2)
            r = 10;
        if (t == RandomType) {
            _lastValue = std::rand() % r;
        }
        else {
            if (_dirUp) {
                if (_lastValue < r) {
                    _lastValue++;
                }
                else {
                    _dirUp = false;
                    _lastValue--;
                }
            }
            else {
                if (_lastValue > 0) {
                    _lastValue--;
                }
                else {
                    _dirUp     = true;
                    _lastValue = 1;
                }
            }
        }
        //
        // Update the current value
        Open62541::Variant v(_lastValue);
        server().writeValue(Value, v);
        //
        MRL::PropertyPath p;
        p.push_back(STOCKDEFS::RuntimeSection);
        MRL::OpcServiceCommon::data().setValue(p, "Value", _lastValue);
        TRC("Last Value " << _lastValue)
        //
        // notify the web interfaces that the values have been updated
        Wt::WServer::instance()->postAll(&SimulatorApp::handleUpdate);
    }
}
