#ifndef SERVEROBJECTTYPE_H
#define SERVEROBJECTTYPE_H
#include "open62541server.h"
namespace Open62541 {

/*!
    \brief The ServerObjectType class
    Object type handling class - this is factory for object type - operates on a server instance
    The NodeContext is the node life cycle manager

*/
class  UA_EXPORT  ServerObjectType {
        Server &_server;
        std::string _name;

        NodeId _typeId;
        int _nameSpace = 1;
    public:
        /*!
            \brief ServerObjectType
            \param s
        */
        ServerObjectType(Server &s, const std::string &n);
        /*!
            \brief ~ServerObjectType
        */
        virtual ~ServerObjectType();
        /*!
            \brief name
            \return
        */
        const std::string &name() {
            return _name;
        }

        /*!
         * \brief nameSpace
         * \return
         */
        int nameSpace() const { return _nameSpace;}
        /*!
         * \brief setNameSpace
         * \param i
         */
        void setNameSpace(int i) { _nameSpace = i;}
        /*!
            \brief server
            \return
        */
        Server &server() {
            return _server;
        }
        /*!
            \brief typeId
            \return
        */
        NodeId &typeId() {
            return _typeId;
        }

        /*!
            \brief addBaseObjectType
            \param n
            \param typeId
            \return
        */
        bool addBaseObjectType(const std::string &n, NodeId &requestNodeId = NodeId::Null,NodeContext *context = nullptr);
        /*!
            \brief addObjectTypeVariable
            \param n
            \param parent
            \param nodeiD
            \param mandatory
            \return
        */
        template<typename T> bool addObjectTypeVariable(const std::string &n, NodeId &parent,
                                                        NodeId &nodeId,
                                                        NodeContext *context = nullptr,
                                                        NodeId &requestNodeId = NodeId::Null, // usually want auto generated ids
                                                        bool mandatory = true) {
            T a = T();
            Variant value(a);
            //
            VariableAttributes var_attr;
            var_attr.setDefault();
            var_attr.setDisplayName(n);
            var_attr.setDescription(n);
            var_attr.get().accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
            var_attr.setValue(value);
            var_attr.get().dataType = value.get().type->typeId;
            QualifiedName qn(_nameSpace,n.c_str());
            if (_server.addVariableNode(requestNodeId, parent, NodeId::HasComponent, qn,
                                        NodeId::BaseDataVariableType, var_attr, nodeId,context)) {
                if (mandatory) {
                    return _server.addReference(nodeId.isNull()?requestNodeId:nodeId,
                                                NodeId::HasModellingRule,
                                                ExpandedNodeId::ModellingRuleMandatory,
                                                true);
                }
                return true;
            }
            UAPRINTLASTERROR(_server.lastError())
            return false;
        }

        /*!
            \brief addDerivedObjectType
            \param server
            \param n
            \param parent
            \param typeId
            \return
        */
        bool addDerivedObjectType(const std::string &n, NodeId &parent, NodeId &typeId,
                                  NodeId &requestNodeId = NodeId::Null, NodeContext *context = nullptr);
        /*!
            \brief addChildren
            \return
        */
        virtual bool addChildren(NodeId &/*parent*/) {
            return true;
        }
        /*!
            \brief addType
            \param server
            \param baseId
            \return
        */
        virtual bool addType(NodeId &nodeId);  // base node of type
        /*!
            \brief append
            \param parent
            \param nodeId
            \return
        */
        virtual bool append(NodeId &parent, NodeId &nodeId, NodeId &requestNodeId = NodeId::Null); // derived type
        /*!
            \brief addInstance
            \param n
            \param parent
            \param nodeId
            \return
        */
        virtual bool addInstance(const std::string &n, NodeId &parent,  NodeId &nodeId,
                                 NodeId &requestNodeId = NodeId::Null, NodeContext *context = nullptr);



};



}
#endif // SERVEROBJECTTYPE_H
