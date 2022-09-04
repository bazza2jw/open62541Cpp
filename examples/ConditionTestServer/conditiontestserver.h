#ifndef CONDITIONTESTSERVER_H
#define CONDITIONTESTSERVER_H
// server object
#include <open62541cpp/open62541server.h>
// node contexts are neded for the read/ write handlers
#include <open62541cpp/nodecontext.h>
#include <open62541cpp/condition.h>

// This condition uses the class methods for call backs
class Condition1 : public Open62541::Condition
{
public:
    Condition1(Open62541::Server& s, const Open62541::NodeId& c, const Open62541::NodeId& src)
        : Condition(s, c, src)
    {
    }
    virtual bool enteringEnabledState()
    {
        std::cerr << "Condition1 " << __FUNCTION__ << std::endl;
        UA_Boolean retain = true;
        server().writeObjectProperty_scalar(condition(), "Retain", &retain, &UA_TYPES[UA_TYPES_BOOLEAN]);
        return true;
    }
    virtual bool enteringAckedState()
    {
        std::cerr << "Condition1 " << __FUNCTION__ << std::endl;
        Open62541::Variant b(false);
        setConditionVariableFieldProperty(b, "ActiveState", "Id");
        return true;
    }
    virtual bool enteringConfirmedState()
    {
        std::cerr << "Condition1 " << __FUNCTION__ << std::endl;
        Open62541::Variant activeStateId(false);

        setConditionVariableFieldProperty(activeStateId, "ActiveState", "Id");
        Open62541::Variant retain(false);
        setConditionField(retain, "Retain");
        return true;
    }
    virtual bool enteringActiveState()
    {
        std::cerr << "Condition1 " << __FUNCTION__ << std::endl;
        return true;
    }
};

/*!
 * \brief The Condition2 class
 */

class Condition2 : public Open62541::Condition
{

public:
    Condition2(Open62541::Server& s, const Open62541::NodeId& c, const Open62541::NodeId& src)
        : Condition(s, c, src)
    {
    }
    virtual bool enteringEnabledState()
    {
        std::cerr << "Condition2 " << __FUNCTION__ << std::endl;
        return true;
    }
    virtual bool enteringAckedState()
    {
        std::cerr << "Condition2 " << __FUNCTION__ << std::endl;
        return true;
    }
    virtual bool enteringConfirmedState()
    {
        std::cerr << "Condition2 " << __FUNCTION__ << std::endl;
        return true;
    }
    virtual bool enteringActiveState()
    {
        std::cerr << "Condition2 " << __FUNCTION__ << std::endl;
        return true;
    }
};

// Node Contexts to handle the varaiable read/ / write notifications
class NodeContextV1 : public Open62541::NodeContext
{
    Open62541::Condition* _condition = nullptr;

public:
    NodeContextV1()
        : NodeContext("V1")
    {
    }
    void setCondition(Open62541::Condition* c) { _condition = c; }
    virtual void writeValue(Open62541::Server& server,
                            Open62541::NodeId& /*node*/,
                            const UA_NumericRange* /*range*/,
                            const UA_DataValue& data)
    {
        std::cerr << "writeValue NodeContextV1" << std::endl;

        if (_condition) {

            server.writeObjectProperty_scalar(_condition->condition(),
                                              "Time",
                                              &data.sourceTimestamp,
                                              &UA_TYPES[UA_TYPES_DATETIME]);

            if (*(UA_Boolean*)(data.value.data) == true) {
                /* By writing "true" in ActiveState/Id, the A&C server will set the
                 * related fields automatically and then will trigger event
                 * notification. */
                Open62541::Variant value(true);
                _condition->setConditionVariableFieldProperty(value, "ActiveState", "Id");
            }
            else {
                /* By writing "false" in ActiveState/Id, the A&C server will set only
                 * the ActiveState field automatically to the value "Inactive". The user
                 * should trigger the event manually by calling
                 * UA_Server_triggerConditionEvent inside the application or call
                 * ConditionRefresh method with client to update the event notification. */
                Open62541::Variant activeStateId(false);
                _condition->setConditionVariableFieldProperty(activeStateId, "ActiveState", "Id");

                _condition->triggerConditionEvent("");
            }
        }
    }
};

class NodeContextV2 : public Open62541::NodeContext
{
    Open62541::Condition* _condition = nullptr;

public:
    NodeContextV2()
        : NodeContext("V2")
    {
    }
    void setCondition(Open62541::Condition* c) { _condition = c; }
    virtual void writeValue(Open62541::Server& server,
                            Open62541::NodeId& /*node*/,
                            const UA_NumericRange* /*range*/,
                            const UA_DataValue& data)
    {
        std::cerr << "writeValue NodeContextV2" << std::endl;
        if (_condition) {
            server.writeObjectProperty_scalar(_condition->condition(),
                                              "Severity",
                                              (UA_UInt16*)data.value.data,
                                              &UA_TYPES[UA_TYPES_UINT16]);
        }
    }
};

class NodeContextV3 : public Open62541::NodeContext
{
    Open62541::Condition* _condition = nullptr;

public:
    NodeContextV3()
        : NodeContext("V3")
    {
    }
    void setCondition(Open62541::Condition* c) { _condition = c; }
    virtual void writeValue(Open62541::Server& server,
                            Open62541::NodeId& /*node*/,
                            const UA_NumericRange* /*range*/,
                            const UA_DataValue& data)
    {
        std::cerr << "writeValue NodeContextV3" << std::endl;
        if (_condition) {

            server.writeObjectProperty_scalar(_condition->condition(),
                                              "Time",
                                              &data.serverTimestamp,
                                              &UA_TYPES[UA_TYPES_DATETIME]);
            Open62541::Variant value(false);
            _condition->setConditionVariableFieldProperty(value, "ActiveState", "Id");

            _condition->setConditionVariableFieldProperty(value, "ConfirmedState", "Id");

            Open62541::Variant severityValue(UA_UInt16(100));
            _condition->setConditionField(severityValue, "Severity");

            Open62541::Variant messageValue("en", "Condition returned to normal state");
            _condition->setConditionField(messageValue, "Message");

            Open62541::Variant commentValue("en", "Normal State");
            _condition->setConditionField(commentValue, "Comment");

            Open62541::Variant retainValue(false);
            _condition->setConditionField(retainValue, "Retain");

            _condition->triggerConditionEvent("");
        }
    }
};

// encapsulate in a server class
/*!
 * \brief The ConditionServer class
 */
class ConditionServer : public Open62541::Server
{
    Open62541::NodeId conditionSource;
    Open62541::Condition_p conditionInstance_1 = nullptr;
    Open62541::Condition_p conditionInstance_2 = nullptr;

    // The node contexts to handle write events
    NodeContextV1 n1;
    NodeContextV2 n2;
    NodeContextV3 n3;

    int _idx;  // name space for this server
public:
    ConditionServer();           // construct the server
    virtual ~ConditionServer();  // destroy
    void initialise();           // pre run preparation
};

#endif  // CONDITIONTESTSERVER_H
