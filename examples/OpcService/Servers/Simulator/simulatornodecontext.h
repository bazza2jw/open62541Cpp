#ifndef SIMULATORNODECONTEXT_H
#define SIMULATORNODECONTEXT_H
#include <Open62541Cpp/open62541server.h>
#include <Open62541Cpp/nodecontext.h>

/*!
 * \brief The SimulatorNodeContext class
 */
class SimulatorNodeContext : public Open62541::NodeContext
{
public:
    SimulatorNodeContext() : Open62541::NodeContext("SimulatorWrite") {}

    virtual ~SimulatorNodeContext() {}
    /*!
        \brief readData
        \param node
        \param range
        \param value
        \return
    */
    virtual bool readData(Open62541::Server &server,  Open62541::NodeId &node, const UA_NumericRange * range, UA_DataValue &value) ;

    /*!
        \brief writeData
        \param server
        \param node
        \param range
        \param value
        \return
    */
    virtual bool writeData(Open62541::Server &server,  Open62541::NodeId &node, const UA_NumericRange * range, const UA_DataValue &value);

};

#endif // SIMULATORNODECONTEXT_H
