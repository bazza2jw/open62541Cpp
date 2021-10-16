#include "conditiontestserver.h"
/*!
 * \brief ConditionServer::ConditionServer
 */
ConditionServer::ConditionServer()  // construct the server
{
}
/*!
 * \brief ConditionServer::~ConditionServer
 */
ConditionServer::~ConditionServer()  // destroy
{
}
/*!
 * \brief ConditionServer::initialise
 */
void ConditionServer::initialise()  // pre run preparation
{

    _idx = addNamespace("urn:test:test");  // create a name space

    Open62541::ObjectAttributes obj;
    obj.setDefault();
    obj.setDisplayName("ConditionSourceObject");
    Open62541::QualifiedName qn(0, "ConditionSourceObject");
    conditionSource.notNull();
    if (!addObjectNode(Open62541::NodeId::Null,
                       Open62541::NodeId::Objects,
                       Open62541::NodeId::Organizes,
                       qn,
                       Open62541::NodeId::BaseObjectType,
                       obj,
                       conditionSource)) {
        std::cerr << "Failed to add source node " << Open62541::toString(lastError()) << std::endl;
        return;
    }

    Open62541::ExpandedNodeId ex1(conditionSource.nameSpaceIndex(), conditionSource.numeric());
    std::string es;
    ex1.toString(es);
    std::cerr << " conditionSource " << es << std::endl;
    if (!addReference(Open62541::NodeId::Server, Open62541::NodeId::HasNotifier, ex1, true)) {
        std::cerr << " Failed to add reference " << Open62541::toString(lastError()) << std::endl;
        return;
    }

    if (!createCondition<Condition1>(UA_NODEID_NUMERIC(0, UA_NS0ID_OFFNORMALALARMTYPE),
                                     "Condition1",
                                     conditionSource,
                                     conditionInstance_1,
                                     Open62541::NodeId::HasComponent)) {
        std::cerr << "Failed to create condition 1" << Open62541::toString(lastError()) << std::endl;
        return;
    }
    // enable the callbacks and handlers
    conditionInstance_1->setEnabled();
    conditionInstance_1->setAcked();
    conditionInstance_1->setConfirmed();
    conditionInstance_1->setActive();

    if (!createCondition<Condition2>(UA_NODEID_NUMERIC(0, UA_NS0ID_OFFNORMALALARMTYPE),
                                     "Condition2",
                                     conditionSource,
                                     conditionInstance_2,
                                     Open62541::NodeId::HasComponent)) {
        std::cerr << "Failed to create condition 2 " << Open62541::toString(lastError()) << std::endl;
        return;
    }

    UA_Boolean retain = UA_TRUE;
    writeObjectProperty_scalar(conditionInstance_2->condition(), "Retain", &retain, &UA_TYPES[UA_TYPES_BOOLEAN]);

    Open62541::Variant value(true);
    if (!conditionInstance_2->setConditionVariableFieldProperty(value, "EnabledState", "Id")) {
        std::cerr << "Failed to setConditionVariableFieldPropert " << Open62541::toString(lastError()) << std::endl;
        return;
    }

    //
    n1.setCondition(conditionInstance_1);
    n2.setCondition(conditionInstance_2);
    n3.setCondition(conditionInstance_1);
    //
    Open62541::NodeId variable_1(_idx, "Activate_Condition_1");
    Open62541::NodeId variable_2(_idx, "Change_Condition_1");
    Open62541::NodeId variable_3(_idx, "Return_Condition_1");
    //
    addVariable(Open62541::NodeId::Objects,
                "Activate Condition 1",
                Open62541::Variant(false),
                variable_1,
                Open62541::NodeId::Null,
                &n1);
    addVariable(Open62541::NodeId::Objects,
                "Change Severity Condition 2",
                Open62541::Variant(false),
                variable_2,
                Open62541::NodeId::Null,
                &n2);
    addVariable(Open62541::NodeId::Objects,
                "Return to Normal Condition 1",
                Open62541::Variant(false),
                variable_3,
                Open62541::NodeId::Null,
                &n3);
    n1.setValueCallback(*this, variable_1);
    n2.setValueCallback(*this, variable_2);
    n3.setValueCallback(*this, variable_3);

    std::cerr << "initalise complete OK" << std::endl;
}
