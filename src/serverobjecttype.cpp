#include "serverobjecttype.h"


/*!
       \brief Open62541::ServerObjectType::ServerObjectType
       \param n
*/
Open62541::ServerObjectType::ServerObjectType(Server &s, const std::string &n) : _server(s),  _name(n) {
}
/*!
    \brief ~ServerObjectType
*/
Open62541::ServerObjectType::~ServerObjectType() {

}

/*!
    \brief addBaseObjectType
    \param n
    \param typeId
    \return
*/
bool Open62541::ServerObjectType::addBaseObjectType(const std::string &n,
                                                    NodeId &requestNodeId,
                                                    NodeContext *context) {
    ObjectTypeAttributes dtAttr;
    QualifiedName qn(_nameSpace, n);
    dtAttr.setDisplayName(n);
    _typeId.notNull();
    return _server.addObjectTypeNode(requestNodeId,
                                     NodeId::BaseObjectType,
                                     NodeId::HasSubType,
                                     qn,
                                     dtAttr,
                                     _typeId,context);
}


/*!
    \brief addDerivedObjectType
    \param n
    \param parent
    \param typeId
    \return
*/
bool Open62541::ServerObjectType::addDerivedObjectType(const std::string &n,
                                                       NodeId &parent,
                                                       NodeId &typeId,
                                                       NodeId &requestNodeId ,
                                                       NodeContext *context) {
    ObjectTypeAttributes ptAttr;
    ptAttr.setDisplayName(n);
    QualifiedName qn(_nameSpace, n);
    //
    return _server.addObjectTypeNode(requestNodeId, parent, NodeId::HasSubType, qn,
                                     ptAttr, typeId,context);
}



/*!
    \brief add
    \param server
    \param baseId
    \return
*/
bool Open62541::ServerObjectType::addType(NodeId &nodeId) { // base node of type
    if (addBaseObjectType(_name, nodeId)) {
        return addChildren(_typeId);
    }
    return false;
}

/*!
    \brief append
    \param server
    \param parent
    \param nodeId
    \return
*/
bool Open62541::ServerObjectType::append(NodeId &parent, NodeId &nodeId, NodeId &requestNodeId) { // derived type - returns node id of append type
    if (addDerivedObjectType(_name, parent, nodeId, requestNodeId)) {
        return addChildren(nodeId);
    }
    return false;
}

/*!
    \brief Open62541::ServerObjectType::addInstance
    \param n
    \param parent
    \param nodeId
    \return
*/
bool Open62541::ServerObjectType::addInstance(const std::string &n, NodeId &parent,
                                              NodeId &nodeId, NodeId &requestNodeId, NodeContext *context) {
    return _server.addInstance(n,
                               requestNodeId,
                               parent,
                               _typeId,
                               nodeId,
                               context);
}




