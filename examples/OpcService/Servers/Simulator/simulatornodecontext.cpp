#include "simulatornodecontext.h"
#include <open62541cpp/open62541objects.h>
#include <OpcServiceCommon/opcservicecommon.h>
#include <OpcServiceCommon/stockdefs.h>
#include "simulatoropc.h"

/*!
    \brief readData
    \param node
    \param range
    \param value
    \return
*/
bool SimulatorNodeContext::readData(Open62541::Server& /*server*/,
                                    Open62541::NodeId& node,
                                    const UA_NumericRange* /*range*/,
                                    UA_DataValue& value)
{
    // get the value to update
    TRC(" Node Id " << Open62541::toString(node));
    if (node.identifierType() == UA_NODEIDTYPE_NUMERIC) {
        value.hasValue = true;
        MRL::PropertyPath path;
        path.push_back(STOCKDEFS::ConfigureSection);
        //
        UA_Int32 v = 0;
        // find in config
        switch (node.get().identifier.numeric) {
            case RangeId:
                path.push_back("Range");
                v = (UA_Int32)(MRL::OpcServiceCommon::data().getValue<double>(path));
                break;
            case TypeId:
                path.push_back("Type");
                v = (UA_Int32)(MRL::OpcServiceCommon::data().getValue<double>(path));
                break;
            case IntervalId:
                path.push_back("Interval");
                v = (UA_Int32)(MRL::OpcServiceCommon::data().getValue<double>(path));
                break;
            default:
                break;
        }
        UA_Variant_setScalarCopy(&value.value, &v, &UA_TYPES[UA_TYPES_INT32]);  // set the value
    }
    return true;
}

/*!
    \brief writeData
    \param server
    \param node
    \param range
    \param value
    \return
*/
bool SimulatorNodeContext::writeData(Open62541::Server& /*server*/,
                                     Open62541::NodeId& node,
                                     const UA_NumericRange* /*range*/,
                                     const UA_DataValue& value)
{
    // get the value to update
    TRC(" Node Id " << Open62541::toString(node));
    if (node.identifierType() == UA_NODEIDTYPE_NUMERIC) {
        if (value.hasValue) {
            if (value.value.type == &UA_TYPES[UA_TYPES_INT32]) {
                MRL::PropertyPath path;
                path.push_back(STOCKDEFS::ConfigureSection);
                //
                UA_Int32* p = (UA_Int32*)(value.value.data);
                double v    = double(*p);

                // find in config
                switch (node.get().identifier.numeric) {
                    case RangeId:
                        path.push_back("Range");
                        MRL::OpcServiceCommon::data().set(path, v);
                        break;
                    case TypeId:
                        path.push_back("Type");
                        MRL::OpcServiceCommon::data().set(path, v);
                        break;
                    case IntervalId:
                        path.push_back("Interval");
                        MRL::OpcServiceCommon::data().set(path, v);
                        break;
                    default:
                        break;
                }
            }
        }
    }
    return true;
}
