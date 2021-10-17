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
    UA_StatusCode _lastError = 0;

public:
    typedef std::function<bool(Condition&)> ConditionFunc;

protected:
    ConditionFunc _enteringEnabledState;
    ConditionFunc _enteringAckedState;
    ConditionFunc _enteringConfirmedState;
    ConditionFunc _enteringActiveState;

public:
    // TO DO add the possible condition type nodes

    Condition(Server& s, const NodeId& c, const NodeId& src);

    virtual ~Condition();

    Server& server() { return _server; }  // owning server

    /* Set the value of condition field.
     *
     * @param server The server object
     * @param condition The NodeId of the node representation of the Condition Instance
     * @param value Variant Value to be written to the Field
     * @param fieldName Name of the Field in which the value should be written
     * @return The StatusCode of the UA_Server_setConditionField method*/
    bool setConditionField(const Variant& v, const std::string& name);
    /*!
     * \brief setConditionVariableFieldProperty
     * \param value
     * \param variableFieldName
     * \param variablePropertyName
     * \return
     */
    bool setConditionVariableFieldProperty(const Variant& value,
                                           const std::string& variableFieldName,
                                           const std::string& variablePropertyName);
    /*!
     * \brief triggerConditionEvent
     * \param outEventId
     * \return
     */
    bool triggerConditionEvent(const std::string& outEventId);
    /*!
     * \brief addConditionOptionalField
     * \param conditionType
     * \param fieldName
     * \param outOptionalVariable
     * \return
     */
    bool addConditionOptionalField(const NodeId& conditionType,
                                   const std::string& fieldName,
                                   NodeId& outOptionalVariable = NodeId::Null);

    /*!
     * \brief twoStateVariableChange
     * \return true if handled ok
     */
    virtual bool twoStateVariableChange() { return false; }
    /*!
        \brief lastError
        \return
    */
    UA_StatusCode lastError() const { return _lastError; }

    /*!
        \brief lastOK
        \return last error code
    */
    bool lastOK() const { return _lastError == UA_STATUSCODE_GOOD; }

    NodeId& condition()
    {
        return _condition;  // condition id
    }
    NodeId& conditionSource()
    {
        return _conditionSource;  // parent of the condition
    }

    // These enable the condition callbacks for the possible types
    bool setCallback(UA_TwoStateVariableCallbackType callbackType, bool removeBranch = false);

    bool setEnabled(bool removeBranch = false) { return setCallback(UA_ENTERING_ENABLEDSTATE, removeBranch); }
    bool setConfirmed(bool removeBranch = false) { return setCallback(UA_ENTERING_CONFIRMEDSTATE, removeBranch); }
    bool setAcked(bool removeBranch = false) { return setCallback(UA_ENTERING_ACKEDSTATE, removeBranch); }
    bool setActive(bool removeBranch = false) { return setCallback(UA_ENTERING_ACTIVESTATE, removeBranch); }

    bool setEnabled(ConditionFunc f, bool removeBranch = false)
    {
        _enteringEnabledState = f;
        return setCallback(UA_ENTERING_ENABLEDSTATE, removeBranch);
    }
    bool setConfirmed(ConditionFunc f, bool removeBranch = false)
    {
        _enteringConfirmedState = f;
        return setCallback(UA_ENTERING_CONFIRMEDSTATE, removeBranch);
    }
    bool setAcked(ConditionFunc f, bool removeBranch = false)
    {
        _enteringAckedState = f;
        return setCallback(UA_ENTERING_ACKEDSTATE, removeBranch);
    }
    bool setActive(ConditionFunc f, bool removeBranch = false)
    {
        _enteringActiveState = f;
        return setCallback(UA_ENTERING_ACTIVESTATE, removeBranch);
    }

protected:
    // Event handlers - default is to use functors if present
    virtual bool enteringEnabledState()
    {
        if (_enteringEnabledState)
            return _enteringEnabledState(*this);
        return false;
    }
    virtual bool enteringAckedState()
    {
        if (_enteringAckedState)
            return _enteringAckedState(*this);
        return false;
    }
    virtual bool enteringConfirmedState()
    {
        if (_enteringConfirmedState)
            return _enteringConfirmedState(*this);
        return false;
    }
    virtual bool enteringActiveState()
    {
        if (_enteringActiveState)
            return _enteringActiveState(*this);
        return false;
    }

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
