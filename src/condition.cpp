

#include <open62541cpp/condition.h>
#include <open62541cpp/open62541server.h>

/*!
 * \brief Open62541::Condition::Condition
 * \param s
 * \param c
 * \param src
 */
Open62541::Condition::Condition(Server& s, const NodeId& c, const NodeId& src)
    : _server(s)
    , _condition(c)
    , _conditionSource(src)
{
}

/*!
 * \brief Open62541::Condition::~Condition
 */
Open62541::Condition::~Condition()
{
    _lastError = UA_Server_deleteCondition(_server.server(), _condition, _conditionSource);
}

/* Set the value of condition field.
 *
 * @param server The server object
 * @param condition The NodeId of the node representation of the Condition Instance
 * @param value Variant Value to be written to the Field
 * @param fieldName Name of the Field in which the value should be written
 * @return The StatusCode of the UA_Server_setConditionField method*/
bool Open62541::Condition::setConditionField(const Variant& v, const std::string& name)
{
    QualifiedName qn(_condition.nameSpaceIndex(), name);
    _lastError = UA_Server_setConditionField(_server, _condition, v, qn);
    return lastOK();
}
/*!
 * \brief Open62541::Condition::setConditionVariableFieldProperty
 * \param value
 * \param variableFieldName
 * \param variablePropertyName
 * \return
 */
bool Open62541::Condition::setConditionVariableFieldProperty(const Variant& value,
                                                             const std::string& variableFieldName,
                                                             const std::string& variablePropertyName)
{

    QualifiedName fn(_condition.nameSpaceIndex(), variableFieldName);
    QualifiedName pn(_condition.nameSpaceIndex(), variablePropertyName);
    _lastError = UA_Server_setConditionVariableFieldProperty(_server, _condition, value, fn, pn);
    return lastOK();
}
/*!
 * \brief Open62541::Condition::triggerConditionEvent
 * \param outEventId
 * \return
 */
bool Open62541::Condition::triggerConditionEvent(const std::string& outEventId)
{
    ByteString b(outEventId);
    _lastError = UA_Server_triggerConditionEvent(_server, _condition, _conditionSource, b);
    return lastOK();
}

/*!
 * \brief Open62541::Condition::addConditionOptionalField
 * \param conditionType
 * \param fieldName
 * \param outOptionalVariable
 * \return
 */
bool Open62541::Condition::addConditionOptionalField(const NodeId& conditionType,
                                                     const std::string& fieldName,
                                                     NodeId& outOptionalVariable)
{
    QualifiedName fn(_condition.nameSpaceIndex(), fieldName);
    _lastError = UA_Server_addConditionOptionalField(_server, _condition, conditionType, fn, outOptionalVariable);
    return lastOK();
}

UA_StatusCode Open62541::Condition::twoStateVariableChangeEnabledStateCallback(UA_Server* server,
                                                                               const UA_NodeId* condition)
{
    Open62541::Server* s = Open62541::Server::findServer(server);
    if (s) {
        ConditionPtr& c = s->findCondition(condition);
        if (c) {
            if (c->enteringEnabledState())
                return UA_STATUSCODE_GOOD;
        }
    }
    return UA_StatusCode(-1);
}
UA_StatusCode Open62541::Condition::twoStateVariableChangeAckedStateCallback(UA_Server* server,
                                                                             const UA_NodeId* condition)
{
    Open62541::Server* s = Open62541::Server::findServer(server);
    if (s) {
        ConditionPtr& c = s->findCondition(condition);
        if (c) {
            if (c->enteringAckedState())
                return UA_STATUSCODE_GOOD;
        }
    }
    return UA_StatusCode(-1);
}
UA_StatusCode Open62541::Condition::twoStateVariableChangeConfirmedStateCallback(UA_Server* server,
                                                                                 const UA_NodeId* condition)
{
    Open62541::Server* s = Open62541::Server::findServer(server);
    if (s) {
        ConditionPtr& c = s->findCondition(condition);
        if (c) {
            if (c->enteringConfirmedState())
                return UA_STATUSCODE_GOOD;
        }
    }
    return UA_StatusCode(-1);
}
UA_StatusCode Open62541::Condition::twoStateVariableChangeActiveStateCallback(UA_Server* server,
                                                                              const UA_NodeId* condition)
{
    Open62541::Server* s = Open62541::Server::findServer(server);
    if (s) {
        ConditionPtr& c = s->findCondition(condition);
        if (c) {
            if (c->enteringActiveState())
                return UA_STATUSCODE_GOOD;
        }
    }
    return UA_StatusCode(-1);
}

/*!
 * \brief Open62541::Condition::setCallback
 * \param callbackType
 * \param removeBranch
 */
bool Open62541::Condition::setCallback(UA_TwoStateVariableCallbackType callbackType, bool removeBranch)
{
    switch (callbackType) {
        case UA_ENTERING_ENABLEDSTATE:
            _lastError =
                UA_Server_setConditionTwoStateVariableCallback(_server.server(),
                                                               _condition,
                                                               _conditionSource,
                                                               (UA_Boolean)removeBranch,
                                                               Condition::twoStateVariableChangeEnabledStateCallback,
                                                               callbackType);
            break;
        case UA_ENTERING_ACKEDSTATE:
            _lastError =
                UA_Server_setConditionTwoStateVariableCallback(_server.server(),
                                                               _condition,
                                                               _conditionSource,
                                                               (UA_Boolean)removeBranch,
                                                               Condition::twoStateVariableChangeAckedStateCallback,
                                                               callbackType);
            break;
        case UA_ENTERING_CONFIRMEDSTATE:
            _lastError =
                UA_Server_setConditionTwoStateVariableCallback(_server.server(),
                                                               _condition,
                                                               _conditionSource,
                                                               (UA_Boolean)removeBranch,
                                                               Condition::twoStateVariableChangeConfirmedStateCallback,
                                                               callbackType);
            break;
        case UA_ENTERING_ACTIVESTATE:
            _lastError =
                UA_Server_setConditionTwoStateVariableCallback(_server.server(),
                                                               _condition,
                                                               _conditionSource,
                                                               (UA_Boolean)removeBranch,
                                                               Condition::twoStateVariableChangeActiveStateCallback,
                                                               callbackType);
            break;
        default:
            break;
    }
    return lastOK();
}
