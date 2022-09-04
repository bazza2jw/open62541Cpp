#ifndef CONDITION_H
#define CONDITION_H
#include <open62541cpp/open62541objects.h>
//
// Use ccmake to enable - enable advanced mode
//
#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
namespace Open62541 {
class Server;
class UA_EXPORT Condition
{
    Server& _server;          // owning server
    NodeId _condition;        // condition id
    NodeId _conditionSource;  // parent of the condition

public:
    typedef std::function<bool(Condition&)> ConditionFunc;

protected:
    ConditionFunc _enteringEnabledState;
    ConditionFunc _enteringAckedState;
    ConditionFunc _enteringConfirmedState;
    ConditionFunc _enteringActiveState;

public:
    // TO DO add the possible condition type nodes

    Condition(Server& s, NodeId c, NodeId src);

    virtual ~Condition();

    Server& server() { return _server; }  // owning server

    /* Set the value of condition field.
     *
     * @param server The server object
     * @param condition The NodeId of the node representation of the Condition Instance
     * @param value Variant Value to be written to the Field
     * @param fieldName Name of the Field in which the value should be written
     * @return The StatusCode of the UA_Server_setConditionField method*/
    void setConditionField(const Variant& v, const std::string& name);
    /*!
     * \brief setConditionVariableFieldProperty
     * \param value
     * \param variableFieldName
     * \param variablePropertyName
     * \return
     */
    void setConditionVariableFieldProperty(const Variant& value,
                                           const std::string& variableFieldName,
                                           const std::string& variablePropertyName);
    /*!
     * \brief triggerConditionEvent
     * \param outEventId
     * \return
     */
    void triggerConditionEvent(const std::string& outEventId);
    /*!
     * \brief addConditionOptionalField
     * \param conditionType
     * \param fieldName
     * \param outOptionalVariable
     * \return
     */
    void addConditionOptionalField(const NodeId& conditionType,
                                   const std::string& fieldName,
                                   NodeId& outOptionalVariable = NodeId::Null);

    /*!
     * \brief twoStateVariableChange
     * \return true if handled ok
     */
    virtual void twoStateVariableChange() {}

    NodeId& condition()
    {
        return _condition;  // condition id
    }
    NodeId& conditionSource()
    {
        return _conditionSource;  // parent of the condition
    }

    // These enable the condition callbacks for the possible types
    void setCallback(UA_TwoStateVariableCallbackType callbackType, bool removeBranch = false);

    void setEnabled(bool removeBranch = false) { setCallback(UA_ENTERING_ENABLEDSTATE, removeBranch); }
    void setConfirmed(bool removeBranch = false) { setCallback(UA_ENTERING_CONFIRMEDSTATE, removeBranch); }
    void setAcked(bool removeBranch = false) { setCallback(UA_ENTERING_ACKEDSTATE, removeBranch); }
    void setActive(bool removeBranch = false) { setCallback(UA_ENTERING_ACTIVESTATE, removeBranch); }

    void setEnabled(ConditionFunc f, bool removeBranch = false)
    {
        _enteringEnabledState = f;
        setCallback(UA_ENTERING_ENABLEDSTATE, removeBranch);
    }
    void setConfirmed(ConditionFunc f, bool removeBranch = false)
    {
        _enteringConfirmedState = f;
        setCallback(UA_ENTERING_CONFIRMEDSTATE, removeBranch);
    }
    void setAcked(ConditionFunc f, bool removeBranch = false)
    {
        _enteringAckedState = f;
        setCallback(UA_ENTERING_ACKEDSTATE, removeBranch);
    }
    void setActive(ConditionFunc f, bool removeBranch = false)
    {
        _enteringActiveState = f;
        setCallback(UA_ENTERING_ACTIVESTATE, removeBranch);
    }

protected:
    // Event handlers - default is to use functors if present
    virtual bool enteringEnabledState() { return _enteringEnabledState && _enteringEnabledState(*this); }
    virtual bool enteringAckedState() { return _enteringAckedState && _enteringAckedState(*this); }
    virtual bool enteringConfirmedState() { return _enteringConfirmedState && _enteringConfirmedState(*this); }
    virtual bool enteringActiveState() { return _enteringActiveState && _enteringActiveState(*this); }

private:
    // the possible callbacks - one for each state transition
    static UA_StatusCode twoStateVariableChangeEnabledStateCallback(UA_Server* server, const UA_NodeId* condition);
    static UA_StatusCode twoStateVariableChangeAckedStateCallback(UA_Server* server, const UA_NodeId* condition);
    static UA_StatusCode twoStateVariableChangeConfirmedStateCallback(UA_Server* server, const UA_NodeId* condition);
    static UA_StatusCode twoStateVariableChangeActiveStateCallback(UA_Server* server, const UA_NodeId* condition);
};

typedef std::unique_ptr<Condition> ConditionPtr;
typedef Condition* Condition_p;
}  // namespace Open62541
#endif
#endif  // CONDITION_H
