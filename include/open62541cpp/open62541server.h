/*
    Copyright (C) 2017 -  B. J. Hill

    This file is part of open62541 C++ classes. open62541 C++ classes are free software: you can
    redistribute it and/or modify it under the terms of the Mozilla Public
    License v2.0 as stated in the LICENSE file provided with open62541.

    open62541 C++ classes are distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
    A PARTICULAR PURPOSE.
*/
#ifndef OPEN62541SERVER_H
#define OPEN62541SERVER_H
#include <open62541cpp/open62541objects.h>
#include <open62541cpp/nodecontext.h>
#include <open62541cpp/servermethod.h>
#include <open62541cpp/serverrepeatedcallback.h>
#include <open62541cpp/condition.h>
#include "exception.h"

namespace Open62541 {

/*!
    \brief The Server class - this abstracts the server side
*/
// This class wraps the corresponding C functions. Refer to the C documentation for a full explanation.
// The main thing to watch for is Node ID objects are passed by reference. There are stock Node Id objects including
// NodeId::Null Pass NodeId::Null where a NULL UA_NodeId pointer is expected. If a NodeId is being passed to receive a
// value use the notNull() method to mark it as a receiver of a new node id. Most functions return true if no error
// happened or throw a StatusCodeException on a bad status code.

class HistoryDataGathering;
class HistoryDataBackend;

/*!
    \brief The Server class
*/
class UA_EXPORT Server
{

public:
    /*!
     * \brief The Timer class - used for timed events
     */
    class Timer
    {
        Server* _server = nullptr;
        UA_UInt64 _id   = 0;
        bool _oneShot   = false;
        std::function<void(Timer&)> _handler;

    public:
        Timer() {}
        Timer(Server* c, UA_UInt64 i, bool os, std::function<void(Timer&)> func)
            : _server(c)
            , _id(i)
            , _oneShot(os)
            , _handler(func)
        {
        }
        virtual ~Timer() { UA_Server_removeCallback(_server->server_or_throw(), _id); }
        virtual void handle()
        {
            if (_handler)
                _handler(*this);
        }
        Server* server() const { return _server; }
        UA_UInt64 id() const { return _id; }
        void setId(UA_UInt64 i) { _id = i; }
        bool oneShot() const { return _oneShot; }
    };

private:
    //
    typedef std::unique_ptr<Timer> TimerPtr;
    std::map<UA_UInt64, TimerPtr> _timerMap;  // one map per client
    UA_Server* _server       = nullptr;       // assume one server per application
    UA_ServerConfig* _config = nullptr;
    UA_Boolean _running      = false;
    ReadWriteMutex _mutex;
    std::string _customHostName;
#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
    std::map<unsigned, ConditionPtr> _conditionMap;  // Conditions - SCADA Alarm state handling by any other name
#endif
    typedef std::map<UA_Server*, Server*> ServerMap;
    static ServerMap _serverMap;                      // Map of servers key by UA_Server pointer
    std::map<UA_UInt64, std::string> _discoveryList;  // set of discovery servers this server has registered with
    std::vector<UA_UsernamePasswordLogin> _logins;    // set of permitted  logins
    //
    static void timerCallback(UA_Server*, void* data)
    {
        // timer callback
        if (data) {
            Timer* t = static_cast<Timer*>(data);
            if (t) {
                t->handle();
                if (t->oneShot()) {
                    // Potential risk of the client disappearing
                    t->server()->_timerMap.erase(t->id());
                }
            }
        }
    }

    /*!
        \brief server
        \return pointer to underlying server structure
    */
    UA_Server* server_or_throw() const
    {
        if (_server)
            return _server;
        throw StringException("Server is not initialized.");
    }

    // Lifecycle call backs
    /* Can be NULL. May replace the nodeContext */
    static UA_StatusCode constructor(UA_Server* server,
                                     const UA_NodeId* sessionId,
                                     void* sessionContext,
                                     const UA_NodeId* nodeId,
                                     void** nodeContext);

    /*  Can be NULL. The context cannot be replaced since the node is destroyed
        immediately afterwards anyway. */
    static void destructor(UA_Server* server,
                           const UA_NodeId* sessionId,
                           void* sessionContext,
                           const UA_NodeId* nodeId,
                           void* nodeContext);
    //

    /* Can be NULL. Called during recursive node instantiation. While mandatory
     * child nodes are automatically created if not already present, optional child
     * nodes are not. This callback can be used to define whether an optional child
     * node should be created.
     *
     * @param server The server executing the callback
     * @param sessionId The identifier of the session
     * @param sessionContext Additional data attached to the session in the
     *        access control layer
     * @param sourceNodeId Source node from the type definition. If the new node
     *        shall be created, it will be a copy of this node.
     * @param targetParentNodeId Parent of the potential new child node
     * @param referenceTypeId Identifies the reference type which that the parent
     *        node has to the new node.
     * @return Return UA_TRUE if the child node shall be instantiatet,
     *         UA_FALSE otherwise. */
    static UA_Boolean createOptionalChildCallback(UA_Server* server,
                                                  const UA_NodeId* sessionId,
                                                  void* sessionContext,
                                                  const UA_NodeId* sourceNodeId,
                                                  const UA_NodeId* targetParentNodeId,
                                                  const UA_NodeId* referenceTypeId);

    /* Can be NULL. Called when a node is to be copied during recursive
     * node instantiation. Allows definition of the NodeId for the new node.
     * If the callback is set to NULL or the resulting NodeId is UA_NODEID_NULL,
     * then a random NodeId will be generated.
     *
     * @param server The server executing the callback
     * @param sessionId The identifier of the session
     * @param sessionContext Additional data attached to the session in the
     *        access control layer
     * @param sourceNodeId Source node of the copy operation
     * @param targetParentNodeId Parent node of the new node
     * @param referenceTypeId Identifies the reference type which that the parent
     *        node has to the new node. */
    static UA_StatusCode generateChildNodeIdCallback(UA_Server* server,
                                                     const UA_NodeId* sessionId,
                                                     void* sessionContext,
                                                     const UA_NodeId* sourceNodeId,
                                                     const UA_NodeId* targetParentNodeId,
                                                     const UA_NodeId* referenceTypeId,
                                                     UA_NodeId* targetNodeId);

    //
    //
    // Access Control Callbacks - these invoke virtual functions to control access

    static void clearAccesControlHandler(UA_AccessControl* ac)
    {
        Server* s = static_cast<Server*>(ac->context);
        if (s) {
            s->clearAccessControl(ac);
        }
    }

    static UA_Boolean allowAddNodeHandler(UA_Server* server,
                                          UA_AccessControl* ac,
                                          const UA_NodeId* sessionId,
                                          void* sessionContext,
                                          const UA_AddNodesItem* item);

    static UA_Boolean allowAddReferenceHandler(UA_Server* server,
                                               UA_AccessControl* ac,
                                               const UA_NodeId* sessionId,
                                               void* sessionContext,
                                               const UA_AddReferencesItem* item);

    static UA_Boolean allowDeleteNodeHandler(UA_Server* server,
                                             UA_AccessControl* ac,
                                             const UA_NodeId* sessionId,
                                             void* sessionContext,
                                             const UA_DeleteNodesItem* item);
    static UA_Boolean allowDeleteReferenceHandler(UA_Server* server,
                                                  UA_AccessControl* ac,
                                                  const UA_NodeId* sessionId,
                                                  void* sessionContext,
                                                  const UA_DeleteReferencesItem* item);
    //
    static UA_StatusCode activateSessionHandler(UA_Server* server,
                                                UA_AccessControl* ac,
                                                const UA_EndpointDescription* endpointDescription,
                                                const UA_ByteString* secureChannelRemoteCertificate,
                                                const UA_NodeId* sessionId,
                                                const UA_ExtensionObject* userIdentityToken,
                                                void** sessionContext);

    /* Deauthenticate a session and cleanup */
    static void closeSessionHandler(UA_Server* server,
                                    UA_AccessControl* ac,
                                    const UA_NodeId* sessionId,
                                    void* sessionContext);

    /* Access control for all nodes*/
    static UA_UInt32 getUserRightsMaskHandler(UA_Server* server,
                                              UA_AccessControl* ac,
                                              const UA_NodeId* sessionId,
                                              void* sessionContext,
                                              const UA_NodeId* nodeId,
                                              void* nodeContext);

    /* Additional access control for variable nodes */
    static UA_Byte getUserAccessLevelHandler(UA_Server* server,
                                             UA_AccessControl* ac,
                                             const UA_NodeId* sessionId,
                                             void* sessionContext,
                                             const UA_NodeId* nodeId,
                                             void* nodeContext);

    /* Additional access control for method nodes */
    static UA_Boolean getUserExecutableHandler(UA_Server* server,
                                               UA_AccessControl* ac,
                                               const UA_NodeId* sessionId,
                                               void* sessionContext,
                                               const UA_NodeId* methodId,
                                               void* methodContext);

    /*  Additional access control for calling a method node in the context of a
        specific object */
    static UA_Boolean getUserExecutableOnObjectHandler(UA_Server* server,
                                                       UA_AccessControl* ac,
                                                       const UA_NodeId* sessionId,
                                                       void* sessionContext,
                                                       const UA_NodeId* methodId,
                                                       void* methodContext,
                                                       const UA_NodeId* objectId,
                                                       void* objectContext);
    /* Allow insert,replace,update of historical data */
    static UA_Boolean allowHistoryUpdateUpdateDataHandler(UA_Server* server,
                                                          UA_AccessControl* ac,
                                                          const UA_NodeId* sessionId,
                                                          void* sessionContext,
                                                          const UA_NodeId* nodeId,
                                                          UA_PerformUpdateType performInsertReplace,
                                                          const UA_DataValue* value);

    /* Allow delete of historical data */
    static UA_Boolean allowHistoryUpdateDeleteRawModifiedHandler(UA_Server* server,
                                                                 UA_AccessControl* ac,
                                                                 const UA_NodeId* sessionId,
                                                                 void* sessionContext,
                                                                 const UA_NodeId* nodeId,
                                                                 UA_DateTime startTimestamp,
                                                                 UA_DateTime endTimestamp,
                                                                 bool isDeleteModified);

    /* Allow browsing a node */
    static UA_Boolean allowBrowseNodeHandler(UA_Server* server,
                                             UA_AccessControl* ac,
                                             const UA_NodeId* sessionId,
                                             void* sessionContext,
                                             const UA_NodeId* nodeId,
                                             void* nodeContext);

#ifdef UA_ENABLE_SUBSCRIPTIONS
    /* Allow transfer of a subscription to another session. The Server shall
     * validate that the Client of that Session is operating on behalf of the
     * same user */
    static UA_Boolean allowTransferSubscriptionHandler(UA_Server* server,
                                                       UA_AccessControl* ac,
                                                       const UA_NodeId* oldSessionId,
                                                       void* oldSessionContext,
                                                       const UA_NodeId* newSessionId,
                                                       void* newSessionContext);
#endif

    // Async handler
    static void asyncOperationNotifyCallback(UA_Server* server);

    /*!
     * \brief monitoredItemRegisterCallback
     * \param server
     * \param sessionId
     * \param sessionContext
     * \param nodeId
     * \param nodeContext
     * \param attibuteId
     * \param removed
     */
    static void monitoredItemRegisterCallback(UA_Server* server,
                                              const UA_NodeId* sessionId,
                                              void* sessionContext,
                                              const UA_NodeId* nodeId,
                                              void* nodeContext,
                                              UA_UInt32 attibuteId,
                                              UA_Boolean removed);

public:
#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
    ConditionPtr& findCondition(const UA_NodeId* condition) { return _conditionMap[UA_NodeId_hash(condition)]; }
    ConditionPtr& findCondition(UA_UInt32 n) { return _conditionMap[n]; }
#endif

public:
    /*!
        \brief Server
    */
    Server()
    {
        _server = UA_Server_new();
        if (!_server)
            throw StringException("Could not create new server instance");
        _config = UA_Server_getConfig(_server);
        if (_config) {
            UA_ServerConfig_setDefault(_config);
            _config->nodeLifecycle.constructor = constructor;  // set up the node global lifecycle
            _config->nodeLifecycle.destructor  = destructor;
        }
    }

    /*!
        \brief Server
        \param port
        \param certificate
    */
    Server(int port, const UA_ByteString& certificate = UA_BYTESTRING_NULL)
    {
        _server = UA_Server_new();
        if (!_server)
            throw StringException("Could not create new server instance");
        _config = UA_Server_getConfig(_server);
        if (_config) {
            UA_ServerConfig_setMinimal(_config, port, &certificate);
            _config->nodeLifecycle.constructor = constructor;  // set up the node global lifecycle
            _config->nodeLifecycle.destructor  = destructor;
        }
    }

    /*!
        \brief ~Server
    */
    virtual ~Server()
    {
        // possible abnormal exit
        if (_server) {
            WriteLock l(_mutex);
            terminate();
        }
    }

    /*!
     * \brief asyncOperationNotify
     * Callback handler
     */
    virtual void asyncOperationNotify() {}

    /*!
     * \brief enableasyncOperationNotify
     */
    void setAsyncOperationNotify()
    {
        if (_config)
            _config->asyncOperationNotifyCallback = Server::asyncOperationNotifyCallback;
    }

    /*!
     * \brief monitoredItemRegister
     * \param sessionId
     * \param sessionContext
     * \param nodeId
     * \param nodeContext
     * \param attibuteId
     * \param removed
     */
    virtual void monitoredItemRegister(const UA_NodeId* /*sessionId*/,
                                       void* /*sessionContext*/,
                                       const UA_NodeId* /*nodeId*/,
                                       void* /*nodeContext*/,
                                       uint32_t /*attibuteId*/,
                                       bool /*removed*/)
    {
    }

    /*!
     * \brief setMonitoredItemRegister
     */
    void setMonitoredItemRegister()
    {
        if (_config)
            _config->monitoredItemRegisterCallback = Server::monitoredItemRegisterCallback;
    }

    /*!
     * \brief createOptionalChild
     * \return true if child is to be created
     */
    UA_Boolean createOptionalChild(const UA_NodeId* /*sessionId*/,
                                   void* /*sessionContext*/,
                                   const UA_NodeId* /*sourceNodeId*/,
                                   const UA_NodeId* /*targetParentNodeId*/,
                                   const UA_NodeId* /*referenceTypeId*/)
    {
        return UA_FALSE;
    }

    /*!
     * \brief setcreateOptionalChild
     */
    void setcreateOptionalChild()
    {
        if (_config)
            _config->nodeLifecycle.createOptionalChild = Server::createOptionalChildCallback;
    }

    /*!
     * \brief generateChildNodeId
     * \param targetNodeId
     * \return UA_STATUSCODE_GOOD on success
     */
    UA_StatusCode generateChildNodeId(const UA_NodeId* /*sessionId*/,
                                      void* /*sessionContext*/,
                                      const UA_NodeId* /*sourceNodeId*/,
                                      const UA_NodeId* /*targetParentNodeId*/,
                                      const UA_NodeId* /*referenceTypeId*/,
                                      UA_NodeId* targetNodeId)
    {
        *targetNodeId = UA_NODEID_NULL;
        std::logic_error("Function not yet implemented");
        return UA_STATUSCODE_BADNOTIMPLEMENTED;
    }

    /*!
     * \brief setGenerateChildNodeId
     */
    void setGenerateChildNodeId()
    {
        if (_config)
            _config->nodeLifecycle.generateChildNodeId = Server::generateChildNodeIdCallback;
    }

    /*!
     * \brief setMdnsServerName
     * \param name
     */

    void setMdnsServerName(const std::string& name)
    {
        if (_config) {
            //_config-> = true;
#ifdef UA_ENABLE_DISCOVERY_MULTICAST
            _config->mdnsConfig.mdnsServerName = UA_String_fromChars(name.c_str());
#else
            (void)name;
#endif
        }
    }

    /*!
        \brief logins
        Array of user name / passowrds - TODO add clear, add, delete update
        \return
    */
    std::vector<UA_UsernamePasswordLogin>& logins() { return _logins; }

    /*!
        \brief applyEndpoints
    */
    void applyEndpoints(EndpointDescriptionArray& endpoints)
    {
        _config->endpoints     = endpoints.data();
        _config->endpointsSize = endpoints.length();
        // Transfer ownership
        endpoints.release();
    }

    /*!
        \brief configClean
    */
    void configClean()
    {
        if (_config)
            UA_ServerConfig_clean(_config);
    }

    /*!
        \brief enableSimpleLogin
        Set up for simple login - assumes the permitted logins have been set up before hand
        This gives username / password access and disables anomyous access
        \return
    */
    bool enableSimpleLogin(bool allowAnonymous = true, const std::string& userTokenPolicyUri = "")
    {
        ByteString ut(userTokenPolicyUri);
        // install access control into the config that maps on to the server hence its virtual functions
        UA_AccessControl_default(_config,
                                 allowAnonymous,
                                 &_config->securityPolicies[_config->securityPoliciesSize - 1].policyUri,
                                 _logins.size(),
                                 _logins.data());
        setAccessControl(&_config->accessControl);  // map access control requests to this object
        return true;
    }

    /* Set a custom hostname in server configuration */

    void setCustomHostname(const std::string& customHostname) { _customHostName = customHostname; }

    /*!
        \brief setServerUri
        \param s
    */
    void setServerUri(const std::string& s)
    {
        UA_String_clear(&_config->applicationDescription.applicationUri);
        _config->applicationDescription.applicationUri = UA_String_fromChars(s.c_str());
    }

    /*!
     * \brief findDataType
     * \param n
     * \return
     */
    const UA_DataType* findDataType(const NodeId& n) { return UA_Server_findDataType(server_or_throw(), n.constRef()); }

    /*!
        \brief findServer
        \param s
        \return
    */
    static Server* findServer(UA_Server* s) { return _serverMap[s]; }
    //
    // Discovery
    //
    /*!
        \brief registerDiscovery
        \param discoveryServerUrl
        \param semaphoreFilePath
        \return true on success
    */
    void registerDiscovery(Client& client, const std::string& semaphoreFilePath = "");

    /*!
        \brief unregisterDiscovery
        \return  true on success
    */
    void unregisterDiscovery(Client& client);

    /*!
        \brief addPeriodicServerRegister
        \param discoveryServerUrl
        \param intervalMs
        \param delayFirstRegisterMs
        \param periodicCallbackId
        \return true on success
    */
    void addPeriodicServerRegister(const std::string& discoveryServerUrl,  // url must persist - that is be static
                                   Client& client,
                                   UA_UInt64& periodicCallbackId,
                                   UA_UInt32 intervalMs           = 600 * 1000,  // default to 10 minutes
                                   UA_UInt32 delayFirstRegisterMs = 1000);

    /*!
        \brief registerServer
    */
    virtual void registerServer(const UA_RegisteredServer* /*registeredServer*/) { OPEN62541_TRC }

    /*!
        \brief registerServerCallback
        \param registeredServer
        \param data
    */
    static void registerServerCallback(const UA_RegisteredServer* registeredServer, void* data);
    /*!
        \brief setRegisterServerCallback
    */
    void setRegisterServerCallback()
    {
        UA_Server_setRegisterServerCallback(server_or_throw(), registerServerCallback, (void*)(this));
    }

    /*!
        \brief serverOnNetwork
        \param serverOnNetwork
        \param isServerAnnounce
        \param isTxtReceived
    */
    virtual void serverOnNetwork(const UA_ServerOnNetwork* /*serverOnNetwork*/,
                                 UA_Boolean /*isServerAnnounce*/,
                                 UA_Boolean /*isTxtReceived*/)
    {
        OPEN62541_TRC
    }

    /*!
        \brief serverOnNetworkCallback
        \param serverNetwork
        \param isServerAnnounce
        \param isTxtReceived
        \param data
    */
    static void serverOnNetworkCallback(const UA_ServerOnNetwork* serverNetwork,
                                        UA_Boolean isServerAnnounce,
                                        UA_Boolean isTxtReceived,
                                        void* data);
#ifdef UA_ENABLE_DISCOVERY_MULTICAST
    /*!
        \brief setServerOnNetworkCallback
    */
    void setServerOnNetworkCallback()
    {
        UA_Server_setServerOnNetworkCallback(server_or_throw(), serverOnNetworkCallback, (void*)(this));
    }
#endif
    /*!
        \brief start
        \param iterate
    */
    virtual void start();  // start the server
    /*!
        \brief stop
    */
    virtual void stop();  // stop the server (prior to delete) - do not try start-stop-start
    /*!
        \brief initialise
    */
    virtual void initialise();  // called after the server object has been created but before run has been called
    /*!
        \brief process
    */
    virtual void process() {}  // called between server loop iterations - hook thread event processing

    /*!
        \brief terminate
    */
    void terminate();  // called before server is closed

    /*!
        \brief server
        \return pointer to underlying server structure
    */
    UA_Server* server() const { return _server; }

    operator UA_Server*() const { return _server; }
    /*!
        \brief running
        \return running state
    */
    UA_Boolean running() const { return _running; }

    /*!
        \brief getNodeContext
        \param n node if
        \param c pointer to context
        \return true on success
    */
    NodeContext* getNodeContext(const NodeId& n) const
    {
        NodeContext* ret = nullptr;
        void* p          = (void*)(ret);
        throw_bad_status(UA_Server_getNodeContext(server_or_throw(), n.get(), &p));
        return ret;
    }

    /*!
        \brief findContext
        \param s name of context
        \return named context
    */
    static NodeContext* findContext(const std::string& s);

    /* Careful! The user has to ensure that the destructor callbacks still work. */
    /*!
        \brief setNodeContext
        \param n node id
        \param c context
        \return true on success
    */
    void setNodeContext(const NodeId& n, const NodeContext* c)
    {
        throw_bad_status(UA_Server_setNodeContext(server_or_throw(), n.get(), (void*)(c)));
    }

    /*!
        \brief readAttribute
        \param nodeId
        \param attributeId
        \param v data pointer
        \return true on success
    */
    void readAttribute(const UA_NodeId* nodeId, UA_AttributeId attributeId, void* v)
    {
        WriteLock l(_mutex);
        throw_bad_status(__UA_Server_read(server_or_throw(), nodeId, attributeId, v));
    }

    /*!
        \brief writeAttribute
        \param nodeId
        \param attributeId
        \param attr_type
        \param attr data pointer
        \return true on success
    */
    void writeAttribute(const UA_NodeId* nodeId,
                        const UA_AttributeId attributeId,
                        const UA_DataType* attr_type,
                        const void* attr)
    {
        WriteLock l(_mutex);
        throw_bad_status(__UA_Server_write(server_or_throw(), nodeId, attributeId, attr_type, attr));
    }
    /*!
        \brief mutex
        \return server mutex
    */
    ReadWriteMutex& mutex()
    {
        return _mutex;  // access mutex - most accesses need a write lock
    }

    /*!
        \brief deleteTree
        \param nodeId node to be delted with its children
        \return true on success
    */
    void deleteTree(const NodeId& nodeId);
    /*!
        \brief browseTree
        \param nodeId  start point
        \param node point in tree to add nodes to
        \return true on success
    */
    void browseTree(const UA_NodeId& nodeId, Open62541::UANode* node);  // add child nodes to property tree node

    /*!
        \brief browseTree
        \param nodeId start point to browse from
        \return true on success
    */
    void browseTree(const NodeId& nodeId,
                    UANodeTree& tree);  // produces an addressable tree using dot seperated browse path
    /*!
        \brief browseTree
        \param nodeId start node to browse from
        \param tree tree to fill
        \return true on success
    */
    void browseTree(const NodeId& nodeId, UANode* tree);
    /*!
        \brief browseTree
        browse and create a map of string version of nodeids ids to node ids
        \param nodeId
        \param tree
        \return true on success
    */
    void browseTree(const NodeId& nodeId, NodeIdMap& m);  //
    /*!
        \brief browseChildren
        \param nodeId parent of childrent ot browse
        \param m map to fill
        \return true on success
    */
    void browseChildren(const UA_NodeId& nodeId, NodeIdMap& m);

    /*  A simplified TranslateBrowsePathsToNodeIds based on the
        SimpleAttributeOperand type (Part 4, 7.4.4.5).

        This specifies a relative path using a list of BrowseNames instead of the
        RelativePath structure. The list of BrowseNames is equivalent to a
        RelativePath that specifies forward references which are subtypes of the
        HierarchicalReferences ReferenceType. All Nodes followed by the browsePath
        shall be of the NodeClass Object or Variable. */
    void browseSimplifiedBrowsePath(const NodeId& origin,
                                    size_t browsePathSize,
                                    const QualifiedName& browsePath,
                                    BrowsePathResult& result)
    {
        result.get() =
            UA_Server_browseSimplifiedBrowsePath(server_or_throw(), origin, browsePathSize, browsePath.constRef());
        throw_bad_status(result.ref()->statusCode);
    }
    /*!
        \brief createBrowsePath
        \param parent node to start with
        \param p path to create
        \param tree
        \return true on success
    */
    bool createBrowsePath(const NodeId& parent,
                          UAPath& p,
                          UANodeTree& tree);  // create a browse path and add it to the tree
    /*!
        \brief addNamespace
        \param s name of name space
        \return name space index
    */
    UA_UInt16 addNamespace(const std::string& s)
    {
        UA_UInt16 ret = 0;
        {
            WriteLock l(mutex());
            ret = UA_Server_addNamespace(server_or_throw(), s.c_str());
        }
        return ret;
    }
    //
    /*!
        \brief serverConfig
        \return  server configuration
    */
    UA_ServerConfig& serverConfig() { return *UA_Server_getConfig(server_or_throw()); }
    //

    /*!
     * \brief addServerMethod
     * \param method - this must persist for the life time of the node !!!!!!
     * \param browseName
     * \param parent
     * \param nodeId
     * \param newNode
     * \param nameSpaceIndex
     * \return
     */
    void addServerMethod(ServerMethod* method,
                         const std::string& browseName,
                         const NodeId& parent,
                         const NodeId& nodeId,
                         NodeId& newNode,
                         int nameSpaceIndex = 0)
    {
        //
        if (nameSpaceIndex == 0) {
            nameSpaceIndex = parent.nameSpaceIndex();  // inherit parent by default
        }
        //
        MethodAttributes attr;
        attr.setDefault();
        attr.setDisplayName(browseName);
        attr.setDescription(browseName);
        attr.setExecutable();
        //
        QualifiedName qn(nameSpaceIndex, browseName);
        {
            WriteLock l(mutex());
            throw_bad_status(
                UA_Server_addMethodNode(server_or_throw(),
                                        nodeId,
                                        parent,
                                        NodeId::HasOrderedComponent,
                                        qn,
                                        attr,
                                        ServerMethod::methodCallback,
                                        method->in().size() - 1,
                                        method->in().data(),
                                        method->out().size() - 1,
                                        method->out().data(),
                                        (void*)(method),  // method context is reference to the call handler
                                        newNode.isNull() ? nullptr : newNode.ref()));
        }
    }

    //
    //
    /*!
        \brief browseName
        \param nodeId
        \return
    */
    void browseName(const NodeId& nodeId, std::string& s, int& ns)
    {
        QualifiedName outBrowseName;
        throw_bad_status(UA_Server_readBrowseName(server_or_throw(), nodeId, outBrowseName));
        s  = toString(outBrowseName.get().name);
        ns = outBrowseName.get().namespaceIndex;
    }

    /*!
        \brief setBrowseName
        \param nodeId
        \param nameSpaceIndex
        \param name
    */
    void setBrowseName(const NodeId& nodeId, int nameSpaceIndex, const std::string& name)
    {
        QualifiedName newBrowseName(nameSpaceIndex, name);
        WriteLock l(_mutex);
        throw_bad_status(UA_Server_writeBrowseName(server_or_throw(), nodeId, newBrowseName));
    }

    /*!
        \brief NodeIdFromPath get the node id from the path of browse names in the given namespace. Tests for node
       existance \param path \param nodeId \return true on success
    */
    bool nodeIdFromPath(const NodeId& start, const Path& path, NodeId& nodeId);

    /*!
        \brief createPath
        Create a path
        \param start
        \param path
        \param nameSpaceIndex
        \param nodeId
        \return true on success
    */
    void createFolderPath(const NodeId& start, const Path& path, int nameSpaceIndex, NodeId& nodeId);

    /*!
        \brief getChild
        \param nameSpaceIndex
        \param childName
        \return true on success
    */
    bool getChild(const NodeId& start, const std::string& childName, NodeId& ret);

    /*!
        \brief addFolder
        \param parent parent node
        \param childName browse name of child node
        \param nodeId  assigned node id or NodeId::Null for auto assign
        \param newNode receives new node if not null
        \param nameSpaceIndex name space index of new node, if non-zero otherwise namespace of parent
        \return true on success
    */
    void addFolder(const NodeId& parent,
                   const std::string& childName,
                   const NodeId& nodeId,
                   NodeId& newNode    = NodeId::Null,
                   int nameSpaceIndex = 0);

    /*!
        \brief addVariable
        \param parent
        \param nameSpaceIndex
        \param childName
        \return true on success
    */
    void addVariable(const NodeId& parent,
                     const std::string& childName,
                     const Variant& value,
                     const NodeId& nodeId,
                     NodeId& newNode    = NodeId::Null,
                     NodeContext* c     = nullptr,
                     int nameSpaceIndex = 0);

    template <typename T>
    /*!
        \brief addVariable
        Add a variable of the given type
        \param parent
        \param childName
        \param nodeId
        \param c
        \param newNode
        \param nameSpaceIndex
        \return true on success
    */
    void addVariable(const NodeId& parent,
                     const std::string& childName,
                     const NodeId& nodeId,
                     const std::string& c,
                     NodeId& newNode    = NodeId::Null,
                     int nameSpaceIndex = 0)
    {
        NodeContext* cp = findContext(c);
        if (!cp)
            throw StringException("Node context not found");
        Variant v((T()));
        addVariable(parent, childName, v, nodeId, newNode, cp, nameSpaceIndex);
    }

    /*!
        \brief addVariable
        \param parent
        \param nameSpaceIndex
        \param childName
        \return true on success
    */
    void addHistoricalVariable(const NodeId& parent,
                               const std::string& childName,
                               const Variant& value,
                               const NodeId& nodeId,
                               NodeId& newNode    = NodeId::Null,
                               NodeContext* c     = nullptr,
                               int nameSpaceIndex = 0);

    template <typename T>
    /*!
        \brief addVariable
        Add a variable of the given type
        \param parent
        \param childName
        \param nodeId
        \param c
        \param newNode
        \param nameSpaceIndex
        \return true on success
    */
    void addHistoricalVariable(const NodeId& parent,
                               const std::string& childName,
                               const NodeId& nodeId,
                               const std::string& c,
                               NodeId& newNode    = NodeId::Null,
                               int nameSpaceIndex = 0)
    {
        NodeContext* cp = findContext(c);
        if (!cp)
            throw StringException("NodeContext not found for string " + c);
        Variant v((T()));
        addHistoricalVariable(parent, childName, v, nodeId, newNode, cp, nameSpaceIndex);
    }

    template <typename T>
    /*!
        \brief addProperty
        Add a property of the given type
        \param parent
        \param key
        \param value
        \param nodeId
        \param newNode
        \param c
        \param nameSpaceIndex
        \return true on success
    */
    void addProperty(const NodeId& parent,
                     const std::string& key,
                     const T& value,
                     const NodeId& nodeId = NodeId::Null,
                     NodeId& newNode      = NodeId::Null,
                     NodeContext* c       = nullptr,
                     int nameSpaceIndex   = 0)
    {
        Variant v(value);
        addProperty(parent, key, v, nodeId, newNode, c, nameSpaceIndex);
    }

    /*!
        \brief addProperty
        \param parent
        \param key
        \param value
        \param nodeId
        \param newNode
        \param c
        \param nameSpaceIndex
        \return true on success
    */
    void addProperty(const NodeId& parent,
                     const std::string& key,
                     const Variant& value,
                     const NodeId& nodeId = NodeId::Null,
                     NodeId& newNode      = NodeId::Null,
                     NodeContext* c       = nullptr,
                     int nameSpaceIndex   = 0);

    /*!
        \brief variable
        \param nodeId
        \param value
        \return true on success
    */
    void variable(const NodeId& nodeId, Variant& value)
    {

        // outValue is managed by caller - transfer to output value
        value.null();
        WriteLock l(_mutex);
        throw_bad_status(UA_Server_readValue(server_or_throw(), nodeId, value.ref()));
    }
    /*!
        \brief deleteNode
        \param nodeId
        \param deleteReferences
        \return true on success
    */
    void deleteNode(const NodeId& nodeId, bool deleteReferences)
    {
        WriteLock l(_mutex);
        throw_bad_status(UA_Server_deleteNode(server_or_throw(), nodeId, UA_Boolean(deleteReferences)));
    }

    /*!
        \brief call
        \param request
        \param ret
        \return true on sucess
    */
    void call(const CallMethodRequest& request, CallMethodResult& ret)
    {
        WriteLock l(_mutex);
        ret.get() = UA_Server_call(server_or_throw(), request);
        throw_bad_status(ret.get().statusCode);
    }

    /*!
        \brief translateBrowsePathToNodeIds
        \param path
        \param result
        \return true on sucess
    */
    void translateBrowsePathToNodeIds(const BrowsePath& path, BrowsePathResult& result)
    {
        WriteLock l(_mutex);
        result.get() = UA_Server_translateBrowsePathToNodeIds(server_or_throw(), path);
        throw_bad_status(result.get().statusCode);
    }

    //
    // Attributes
    //
    /*!
        \brief readNodeId
        \param nodeId
        \param outNodeId
        \return true on sucess
    */
    void readNodeId(const NodeId& nodeId, NodeId& outNodeId)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_NODEID, outNodeId);
    }
    /*!
        \brief readNodeClass
        \param nodeId
        \param outNodeClass
        \return true on success
    */
    void readNodeClass(const NodeId& nodeId, UA_NodeClass& outNodeClass)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_NODECLASS, &outNodeClass);
    }
    /*!
        \brief readBrowseName
        \param nodeId
        \param outBrowseName
        \return true on success
    */
    void readBrowseName(const NodeId& nodeId, QualifiedName& outBrowseName)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_BROWSENAME, outBrowseName);
    }
    /*!
        \brief readDisplayName
        \param nodeId
        \param outDisplayName
        \return true on sucess
    */
    void readDisplayName(const NodeId& nodeId, LocalizedText& outDisplayName)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_DISPLAYNAME, outDisplayName);
    }
    /*!
        \brief readDescription
        \param nodeId
        \param outDescription
        \return true on success
    */
    void readDescription(const NodeId& nodeId, LocalizedText& outDescription)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_DESCRIPTION, outDescription);
    }
    /*!
        \brief readWriteMask
        \param nodeId
        \param outWriteMask
        \return true on sucess
    */
    void readWriteMask(const NodeId& nodeId, UA_UInt32& outWriteMask)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_WRITEMASK, &outWriteMask);
    }
    /*!
        \brief readIsAbstract
        \param nodeId
        \param outIsAbstract
        \return true on success
    */
    void readIsAbstract(const NodeId& nodeId, UA_Boolean& outIsAbstract)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_ISABSTRACT, &outIsAbstract);
    }
    /*!
        \brief readSymmetric
        \param nodeId
        \param outSymmetric
        \return true on success
    */
    void readSymmetric(const NodeId& nodeId, UA_Boolean& outSymmetric)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_SYMMETRIC, &outSymmetric);
    }
    /*!
        \brief readInverseName
        \param nodeId
        \param outInverseName
        \return true on success
    */
    void readInverseName(const NodeId& nodeId, LocalizedText& outInverseName)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_INVERSENAME, outInverseName);
    }
    /*!
        \brief readContainsNoLoop
        \param nodeId
        \param outContainsNoLoops
        \return true on success
    */
    void readContainsNoLoop(const NodeId& nodeId, UA_Boolean& outContainsNoLoops)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_CONTAINSNOLOOPS, &outContainsNoLoops);
    }
    /*!
        \brief readEventNotifier
        \param nodeId
        \param outEventNotifier
        \return
    */
    void readEventNotifier(const NodeId& nodeId, UA_Byte& outEventNotifier)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_EVENTNOTIFIER, &outEventNotifier);
    }
    /*!
        \brief readValue
        \param nodeId
        \param outValue
        \return
    */
    void readValue(const NodeId& nodeId, Variant& outValue) { readAttribute(nodeId, UA_ATTRIBUTEID_VALUE, outValue); }
    /*!
        \brief readDataType
        \param nodeId
        \param outDataType
        \return
    */
    void readDataType(const NodeId& nodeId, NodeId& outDataType)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_DATATYPE, outDataType);
    }
    /*!
        \brief readValueRank
        \param nodeId
        \param outValueRank
        \return
    */
    void readValueRank(const NodeId& nodeId, UA_Int32& outValueRank)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_VALUERANK, &outValueRank);
    }

    /* Returns a variant with an int32 array */
    /*!
        \brief readArrayDimensions
        \param nodeId
        \param outArrayDimensions
        \return
    */
    void readArrayDimensions(const NodeId& nodeId, Variant& outArrayDimensions)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_ARRAYDIMENSIONS, outArrayDimensions);
    }
    /*!
        \brief readAccessLevel
        \param nodeId
        \param outAccessLevel
        \return
    */
    void readAccessLevel(const NodeId& nodeId, UA_Byte& outAccessLevel)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_ACCESSLEVEL, &outAccessLevel);
    }
    /*!
        \brief readMinimumSamplingInterval
        \param nodeId
        \param outMinimumSamplingInterval
        \return
    */
    void readMinimumSamplingInterval(const NodeId& nodeId, UA_Double& outMinimumSamplingInterval)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_MINIMUMSAMPLINGINTERVAL, &outMinimumSamplingInterval);
    }
    /*!
        \brief readHistorizing
        \param nodeId
        \param outHistorizing
        \return
    */
    void readHistorizing(const NodeId& nodeId, UA_Boolean& outHistorizing)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_HISTORIZING, &outHistorizing);
    }
    /*!
        \brief readExecutable
        \param nodeId
        \param outExecutable
        \return
    */
    void readExecutable(const NodeId& nodeId, UA_Boolean& outExecutable)
    {
        readAttribute(nodeId, UA_ATTRIBUTEID_EXECUTABLE, &outExecutable);
    }

    void readObjectProperty(const NodeId& objectId, const QualifiedName& propertyName, Variant& value)
    {
        throw_bad_status(UA_Server_readObjectProperty(server_or_throw(), objectId, propertyName, value));
    }
    /*!
        \brief writeBrowseName
        \param nodeId
        \param browseName
        \return
    */
    void writeBrowseName(const NodeId& nodeId, QualifiedName& browseName)
    {
        writeAttribute(nodeId, UA_ATTRIBUTEID_BROWSENAME, &UA_TYPES[UA_TYPES_QUALIFIEDNAME], browseName);
    }
    /*!
        \brief writeDisplayName
        \param nodeId
        \param displayName
        \return
    */
    void writeDisplayName(const NodeId& nodeId, LocalizedText& displayName)
    {
        writeAttribute(nodeId, UA_ATTRIBUTEID_DISPLAYNAME, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT], displayName);
    }
    /*!
        \brief writeDescription
        \param nodeId
        \param description
        \return
    */
    void writeDescription(const NodeId& nodeId, LocalizedText& description)
    {
        writeAttribute(nodeId, UA_ATTRIBUTEID_DESCRIPTION, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT], description);
    }
    /*!
        \brief writeWriteMask
        \param nodeId
        \param writeMask
        \return
    */
    void writeWriteMask(const NodeId& nodeId, const UA_UInt32 writeMask)
    {
        writeAttribute(nodeId, UA_ATTRIBUTEID_WRITEMASK, &UA_TYPES[UA_TYPES_UINT32], &writeMask);
    }
    /*!
        \brief writeIsAbstract
        \param nodeId
        \param isAbstract
        \return
    */
    void writeIsAbstract(const NodeId& nodeId, const UA_Boolean isAbstract)
    {
        writeAttribute(nodeId, UA_ATTRIBUTEID_ISABSTRACT, &UA_TYPES[UA_TYPES_BOOLEAN], &isAbstract);
    }
    /*!
        \brief writeInverseName
        \param nodeId
        \param inverseName
        \return
    */
    void writeInverseName(const NodeId& nodeId, const UA_LocalizedText inverseName)
    {
        writeAttribute(nodeId, UA_ATTRIBUTEID_INVERSENAME, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT], &inverseName);
    }
    /*!
        \brief writeEventNotifier
        \param nodeId
        \param eventNotifier
        \return
    */
    void writeEventNotifier(const NodeId& nodeId, const UA_Byte eventNotifier)
    {
        writeAttribute(nodeId, UA_ATTRIBUTEID_EVENTNOTIFIER, &UA_TYPES[UA_TYPES_BYTE], &eventNotifier);
    }
    /*!
        \brief writeValue
        \param nodeId
        \param value
        \return
    */
    void writeValue(const NodeId& nodeId, const Variant& value)
    {
        throw_bad_status(
            __UA_Server_write(server_or_throw(), nodeId, UA_ATTRIBUTEID_VALUE, &UA_TYPES[UA_TYPES_VARIANT], value));
    }
    /*!
        \brief writeDataType
        \param nodeId
        \param dataType
        \return
    */
    void writeDataType(const NodeId& nodeId, const NodeId& dataType)
    {
        writeAttribute(nodeId, UA_ATTRIBUTEID_DATATYPE, &UA_TYPES[UA_TYPES_NODEID], dataType);
    }
    /*!
        \brief writeValueRank
        \param nodeId
        \param valueRank
        \return
    */
    void writeValueRank(const NodeId& nodeId, const UA_Int32 valueRank)
    {
        writeAttribute(nodeId, UA_ATTRIBUTEID_VALUERANK, &UA_TYPES[UA_TYPES_INT32], &valueRank);
    }

    /*!
        \brief writeArrayDimensions
        \param nodeId
        \param arrayDimensions
        \return
    */
    void writeArrayDimensions(const NodeId& nodeId, const Variant arrayDimensions)
    {
        writeAttribute(nodeId, UA_ATTRIBUTEID_VALUE, &UA_TYPES[UA_TYPES_VARIANT], arrayDimensions.constRef());
    }
    /*!
        \brief writeAccessLevel
        \param nodeId
        \param accessLevel
        \return
    */
    void writeAccessLevel(const NodeId& nodeId, const UA_Byte accessLevel)
    {
        writeAttribute(nodeId, UA_ATTRIBUTEID_ACCESSLEVEL, &UA_TYPES[UA_TYPES_BYTE], &accessLevel);
    }

    // Some short cuts
    /*!
        \brief writeEnable
        \param nodeId
        \return
    */
    void writeEnable(const NodeId& nodeId)
    {
        UA_Byte accessLevel;
        readAccessLevel(nodeId, accessLevel);
        accessLevel |= UA_ACCESSLEVELMASK_WRITE;
        writeAccessLevel(nodeId, accessLevel);
    }
    /*!
        \brief setReadOnly
        \param nodeId
        \param historyEnable
        \return
    */
    void setReadOnly(const NodeId& nodeId, bool historyEnable = false)
    {
        UA_Byte accessLevel;
        readAccessLevel(nodeId, accessLevel);
        // remove the write bits
        accessLevel &= ~(UA_ACCESSLEVELMASK_WRITE | UA_ACCESSLEVELMASK_HISTORYWRITE);
        // add the read bits
        accessLevel |= UA_ACCESSLEVELMASK_READ;
        if (historyEnable) {
            accessLevel |= UA_ACCESSLEVELMASK_HISTORYREAD;
        }
        writeAccessLevel(nodeId, accessLevel);
    }

    /*!
        \brief writeMinimumSamplingInterval
        \param nodeId
        \param miniumSamplingInterval
        \return
    */
    void writeMinimumSamplingInterval(const NodeId& nodeId, const UA_Double miniumSamplingInterval)
    {
        writeAttribute(nodeId,
                       UA_ATTRIBUTEID_MINIMUMSAMPLINGINTERVAL,
                       &UA_TYPES[UA_TYPES_DOUBLE],
                       &miniumSamplingInterval);
    }
    /*!
        \brief writeExecutable
        \param nodeId
        \param executable
        \return
    */
    void writeExecutable(const NodeId& nodeId, const UA_Boolean executable)
    {
        writeAttribute(nodeId, UA_ATTRIBUTEID_EXECUTABLE, &UA_TYPES[UA_TYPES_BOOLEAN], &executable);
    }

    /*!
     * \brief writeObjectProperty
     * \param objectId
     * \param propertyName
     * \param value
     * \return true on success
     */
    void writeObjectProperty(const NodeId& objectId, const QualifiedName& propertyName, const Variant& value)
    {
        throw_bad_status(UA_Server_writeObjectProperty(server_or_throw(), objectId, propertyName, value));
    }

    template <typename T>
    /*!
     * \brief writeObjectProperty
     * \param objectId
     * \param propertyName
     * \param value
     * \return
     */
    void writeObjectProperty(const NodeId& objectId, const std::string& propertyName, const T& value)
    {
        Variant v(value);
        QualifiedName qn(0, propertyName);
        writeObjectProperty(objectId, qn, v);
    }
    /*!
     * \brief writeObjectProperty_scalar
     * \param objectId
     * \param propertyName
     * \param value
     * \param type
     * \return true on success
     */
    void writeObjectProperty_scalar(const NodeId& objectId,
                                    const std::string& propertyName,
                                    const void* value,
                                    const UA_DataType* type)
    {
        QualifiedName qn(0, propertyName);
        throw_bad_status(UA_Server_writeObjectProperty_scalar(server_or_throw(), objectId, qn, value, type));
    }

    //
    // Add Nodes - taken from docs
    //
    /*!
        \brief addVariableNode
        \param requestedNewNodeId
        \param parentNodeId
        \param referenceTypeId
        \param browseName
        \param typeDefinition
        \param attr
        \param outNewNodeId
        \param instantiationCallback
        \return
    */
    void addVariableNode(const NodeId& requestedNewNodeId,
                         const NodeId& parentNodeId,
                         const NodeId& referenceTypeId,
                         const QualifiedName& browseName,
                         const NodeId& typeDefinition,
                         const VariableAttributes& attr,
                         NodeId& outNewNodeId = NodeId::Null,
                         NodeContext* nc      = nullptr)
    {
        WriteLock l(_mutex);
        throw_bad_status(UA_Server_addVariableNode(server_or_throw(),
                                                   requestedNewNodeId,
                                                   parentNodeId,
                                                   referenceTypeId,
                                                   browseName,
                                                   typeDefinition,
                                                   attr,
                                                   nc,
                                                   outNewNodeId.isNull() ? nullptr : outNewNodeId.ref()));
    }

    /*!
        \brief addVariableTypeNode
        \param requestedNewNodeId
        \param parentNodeId
        \param referenceTypeId
        \param browseName
        \param typeDefinition
        \param attr
        \param outNewNodeId
        \param instantiationCallback
        \return
    */
    void addVariableTypeNode(const NodeId& requestedNewNodeId,
                             const NodeId& parentNodeId,
                             const NodeId& referenceTypeId,
                             const QualifiedName& browseName,
                             const NodeId& typeDefinition,
                             const VariableTypeAttributes& attr,
                             NodeId& outNewNodeId               = NodeId::Null,
                             NodeContext* instantiationCallback = nullptr)
    {
        WriteLock l(_mutex);
        throw_bad_status(UA_Server_addVariableTypeNode(server_or_throw(),
                                                       requestedNewNodeId,
                                                       parentNodeId,
                                                       referenceTypeId,
                                                       browseName,
                                                       typeDefinition,
                                                       attr,
                                                       instantiationCallback,
                                                       outNewNodeId.isNull() ? nullptr : outNewNodeId.ref()));
    }

    /*!
        \brief addObjectNode
        \param requestedNewNodeId
        \param parentNodeId
        \param referenceTypeId
        \param browseName
        \param typeDefinition
        \param attr
        \param outNewNodeId
        \param instantiationCallback
        \return
    */
    void addObjectNode(const NodeId& requestedNewNodeId,
                       const NodeId& parentNodeId,
                       const NodeId& referenceTypeId,
                       const QualifiedName& browseName,
                       const NodeId& typeDefinition,
                       const ObjectAttributes& attr,
                       NodeId& outNewNodeId               = NodeId::Null,
                       NodeContext* instantiationCallback = nullptr)
    {

        WriteLock l(_mutex);
        throw_bad_status(UA_Server_addObjectNode(server_or_throw(),
                                                 requestedNewNodeId,
                                                 parentNodeId,
                                                 referenceTypeId,
                                                 browseName,
                                                 typeDefinition,
                                                 attr,
                                                 instantiationCallback,
                                                 outNewNodeId.isNull() ? nullptr : outNewNodeId.clearRef()));
    }

    /*!
        \brief addObjectTypeNode
        \param requestedNewNodeId
        \param parentNodeId
        \param referenceTypeId
        \param browseName
        \param attr
        \param outNewNodeId
        \param instantiationCallback
        \return
    */
    void addObjectTypeNode(const NodeId& requestedNewNodeId,
                           const NodeId& parentNodeId,
                           const NodeId& referenceTypeId,
                           const QualifiedName& browseName,
                           const ObjectTypeAttributes& attr,
                           NodeId& outNewNodeId               = NodeId::Null,
                           NodeContext* instantiationCallback = nullptr)
    {
        throw_bad_status(UA_Server_addObjectTypeNode(server_or_throw(),
                                                     requestedNewNodeId,
                                                     parentNodeId,
                                                     referenceTypeId,
                                                     browseName,
                                                     attr,
                                                     instantiationCallback,
                                                     outNewNodeId.isNull() ? nullptr : outNewNodeId.clearRef()));
    }

    /*!
        \brief addViewNode
        \param requestedNewNodeId
        \param parentNodeId
        \param referenceTypeId
        \param browseName
        \param attr
        \param outNewNodeId
        \param instantiationCallback
        \return
    */
    void addViewNode(const NodeId& requestedNewNodeId,
                     const NodeId& parentNodeId,
                     const NodeId& referenceTypeId,
                     const QualifiedName& browseName,
                     const ViewAttributes& attr,
                     NodeId& outNewNodeId               = NodeId::Null,
                     NodeContext* instantiationCallback = nullptr)
    {
        WriteLock l(_mutex);
        throw_bad_status(UA_Server_addViewNode(server_or_throw(),
                                               requestedNewNodeId,
                                               parentNodeId,
                                               referenceTypeId,
                                               browseName,
                                               attr,
                                               instantiationCallback,
                                               outNewNodeId.isNull() ? nullptr : outNewNodeId.clearRef()));
    }

    /*!
        \brief addReferenceTypeNode
        \param requestedNewNodeId
        \param parentNodeId
        \param referenceTypeId
        \param browseName
        \param attr
        \param outNewNodeId
        \param instantiationCallback
        \return
    */
    void addReferenceTypeNode(const NodeId& requestedNewNodeId,
                              const NodeId& parentNodeId,
                              const NodeId& referenceTypeId,
                              const QualifiedName& browseName,
                              const ReferenceTypeAttributes& attr,
                              NodeId& outNewNodeId               = NodeId::Null,
                              NodeContext* instantiationCallback = nullptr)
    {
        WriteLock l(_mutex);
        throw_bad_status(UA_Server_addReferenceTypeNode(server_or_throw(),
                                                        requestedNewNodeId,
                                                        parentNodeId,
                                                        referenceTypeId,
                                                        browseName,
                                                        attr,
                                                        instantiationCallback,
                                                        outNewNodeId.isNull() ? nullptr : outNewNodeId.clearRef()));
    }

    /*!
        \brief addDataTypeNode
        \param requestedNewNodeId
        \param parentNodeId
        \param referenceTypeId
        \param browseName
        \param attr
        \param outNewNodeId
        \param instantiationCallback
        \return
    */
    void addDataTypeNode(const NodeId& requestedNewNodeId,
                         const NodeId& parentNodeId,
                         const NodeId& referenceTypeId,
                         const QualifiedName& browseName,
                         const DataTypeAttributes& attr,
                         NodeId& outNewNodeId               = NodeId::Null,
                         NodeContext* instantiationCallback = nullptr)
    {
        WriteLock l(_mutex);
        throw_bad_status(UA_Server_addDataTypeNode(server_or_throw(),
                                                   requestedNewNodeId,
                                                   parentNodeId,
                                                   referenceTypeId,
                                                   browseName,
                                                   attr,
                                                   instantiationCallback,
                                                   outNewNodeId.isNull() ? nullptr : outNewNodeId.clearRef()));
    }

    /*!
        \brief addDataSourceVariableNode
        \param requestedNewNodeId
        \param parentNodeId
        \param referenceTypeId
        \param browseName
        \param typeDefinition
        \param attr
        \param dataSource
        \param outNewNodeId
        \return
    */
    void addDataSourceVariableNode(const NodeId& requestedNewNodeId,
                                   const NodeId& parentNodeId,
                                   const NodeId& referenceTypeId,
                                   const QualifiedName& browseName,
                                   const NodeId& typeDefinition,
                                   const VariableAttributes& attr,
                                   const DataSource& dataSource,
                                   NodeId& outNewNodeId               = NodeId::Null,
                                   NodeContext* instantiationCallback = nullptr)
    {
        WriteLock l(_mutex);
        throw_bad_status(UA_Server_addDataSourceVariableNode(server_or_throw(),
                                                             requestedNewNodeId,
                                                             parentNodeId,
                                                             referenceTypeId,
                                                             browseName,
                                                             typeDefinition,
                                                             attr,
                                                             dataSource,
                                                             instantiationCallback,
                                                             outNewNodeId.isNull() ? nullptr : outNewNodeId.ref()));
    }

    /*!
        \brief addReference
        \param sourceId
        \param refTypeId
        \param targetId
        \param isForward
        \return
    */
    void addReference(const NodeId& sourceId, const NodeId& refTypeId, const ExpandedNodeId& targetId, bool isForward)
    {
        WriteLock l(_mutex);
        throw_bad_status(UA_Server_addReference(server_or_throw(), sourceId, refTypeId, targetId, isForward));
    }

    /*!
        \brief markMandatory
        \param nodeId
        \return
    */
    void markMandatory(const NodeId& nodeId)
    {
        addReference(nodeId, NodeId::HasModellingRule, ExpandedNodeId::ModellingRuleMandatory, true);
    }

    /*!
        \brief deleteReference
        \param sourceNodeId
        \param referenceTypeId
        \param isForward
        \param targetNodeId
        \param deleteBidirectional
        \return
    */
    void deleteReference(const NodeId& sourceNodeId,
                         const NodeId& referenceTypeId,
                         bool isForward,
                         const ExpandedNodeId& targetNodeId,
                         bool deleteBidirectional)
    {

        WriteLock l(_mutex);
        throw_bad_status(UA_Server_deleteReference(server_or_throw(),
                                                   sourceNodeId,
                                                   referenceTypeId,
                                                   isForward,
                                                   targetNodeId,
                                                   deleteBidirectional));
    }

    /*!
        \brief Open62541::Server::addInstance
        \param n
        \param parent
        \param nodeId
        \return
    */
    void addInstance(const std::string& n,
                     const NodeId& requestedNewNodeId,
                     const NodeId& parent,
                     const NodeId& typeId,
                     NodeId& nodeId       = NodeId::Null,
                     NodeContext* context = nullptr)
    {
        ObjectAttributes oAttr;
        oAttr.setDefault();
        oAttr.setDisplayName(n);
        QualifiedName qn(parent.nameSpaceIndex(), n);
        addObjectNode(requestedNewNodeId, parent, NodeId::Organizes, qn, typeId, oAttr, nodeId, context);
    }
    //
    //
    //
    /*  Creates a node representation of an event

        @param server The server object
        @param eventType The type of the event for which a node should be created
        @param outNodeId The NodeId of the newly created node for the event
        @return The StatusCode of the UA_Server_createEvent method */
    void createEvent(const NodeId& eventType, NodeId& outNodeId)
    {
        WriteLock l(_mutex);
        throw_bad_status(UA_Server_createEvent(server_or_throw(), eventType, outNodeId.ref()));
    }

    /*  Triggers a node representation of an event by applying EventFilters and
        adding the event to the appropriate queues.
        @param server The server object
        @param eventNodeId The NodeId of the node representation of the event which should be triggered
        @param outEvent the EventId of the new event
        @param deleteEventNode Specifies whether the node representation of the event should be deleted
        @return The StatusCode of the UA_Server_triggerEvent method */
    void triggerEvent(const NodeId& eventNodeId,
                      const NodeId& sourceNode,
                      UA_ByteString* outEventId = nullptr,
                      bool deleteEventNode      = true)
    {
        WriteLock l(_mutex);
        throw_bad_status(
            UA_Server_triggerEvent(server_or_throw(), eventNodeId, sourceNode, outEventId, deleteEventNode));
    }

    /*!
     \brief addNewEventType
     \param name
     \param description
     \param eventType  the event type node
     \return true on success
 */
    void addNewEventType(const std::string& name, NodeId& eventType, const std::string& description = std::string())
    {
        ObjectTypeAttributes attr;
        attr.setDefault();
        attr.setDisplayName(name);
        attr.setDescription((description.empty() ? name : description));
        QualifiedName qn(0, name);
        WriteLock l(_mutex);
        throw_bad_status(UA_Server_addObjectTypeNode(server_or_throw(),
                                                     UA_NODEID_NULL,
                                                     UA_NODEID_NUMERIC(0, UA_NS0ID_BASEEVENTTYPE),
                                                     UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
                                                     qn,
                                                     attr,
                                                     NULL,
                                                     eventType.clearRef()));
    }

    /*!
        \brief setUpEvent
        \param outId
        \param eventMessage
        \param eventSourceName
        \param eventSeverity
        \param eventTime
        \return true on success
    */

    void setUpEvent(NodeId& outId,
                    const NodeId& eventType,
                    const std::string& eventMessage,
                    const std::string& eventSourceName,
                    int eventSeverity     = 100,
                    UA_DateTime eventTime = UA_DateTime_now())
    {
        WriteLock l(_mutex);
        outId.notNull();
        throw_bad_status(UA_Server_createEvent(server_or_throw(), eventType, outId));

        /* Set the Event Attributes */
        /* Setting the Time is required or else the event will not show up in UAExpert! */
        throw_bad_status(UA_Server_writeObjectProperty_scalar(server_or_throw(),
                                                              outId,
                                                              UA_QUALIFIEDNAME(0, const_cast<char*>("Time")),
                                                              &eventTime,
                                                              &UA_TYPES[UA_TYPES_DATETIME]));

        throw_bad_status(UA_Server_writeObjectProperty_scalar(server_or_throw(),
                                                              outId,
                                                              UA_QUALIFIEDNAME(0, const_cast<char*>("Severity")),
                                                              &eventSeverity,
                                                              &UA_TYPES[UA_TYPES_UINT16]));

        LocalizedText eM(const_cast<char*>("en-US"), eventMessage);
        throw_bad_status(UA_Server_writeObjectProperty_scalar(server_or_throw(),
                                                              outId,
                                                              UA_QUALIFIEDNAME(0, const_cast<char*>("Message")),
                                                              eM.clearRef(),
                                                              &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]));

        UA_String eSN = UA_STRING(const_cast<char*>(eventSourceName.c_str()));
        throw_bad_status(UA_Server_writeObjectProperty_scalar(server_or_throw(),
                                                              outId,
                                                              UA_QUALIFIEDNAME(0, const_cast<char*>("SourceName")),
                                                              &eSN,
                                                              &UA_TYPES[UA_TYPES_STRING]));
    }

    /*!
        \brief updateCertificate
        \param oldCertificate
        \param newCertificate
        \param newPrivateKey
        \param closeSessions
        \param closeSecureChannels
        \return true on success
    */
    void updateCertificate(const UA_ByteString* oldCertificate,
                           const UA_ByteString* newCertificate,
                           const UA_ByteString* newPrivateKey,
                           bool closeSessions       = true,
                           bool closeSecureChannels = true)
    {
        WriteLock l(_mutex);
        throw_bad_status(UA_Server_updateCertificate(server_or_throw(),
                                                     oldCertificate,
                                                     newCertificate,
                                                     newPrivateKey,
                                                     closeSessions,
                                                     closeSecureChannels));
    }

    /*!
     * \brief accessControl
     * \return
     */
    UA_AccessControl& accessControl()
    {
        if (!(server() && _config))
            throw std::runtime_error("Invalid server / config");
        return _config->accessControl;
    }

    /*!
     * \brief setAccessControl
     */
    virtual void setAccessControl(UA_AccessControl* ac)
    {
        ac->activateSession                     = Server::activateSessionHandler;
        ac->allowAddNode                        = Server::allowAddNodeHandler;
        ac->allowAddReference                   = Server::allowAddReferenceHandler;
        ac->allowBrowseNode                     = Server::allowBrowseNodeHandler;
        ac->allowDeleteNode                     = Server::allowDeleteNodeHandler;
        ac->allowDeleteReference                = Server::allowDeleteReferenceHandler;
        ac->allowHistoryUpdateDeleteRawModified = Server::allowHistoryUpdateDeleteRawModifiedHandler;
        ac->allowHistoryUpdateUpdateData        = Server::allowHistoryUpdateUpdateDataHandler;
        ac->allowTransferSubscription           = Server::allowTransferSubscriptionHandler;
        ac->clear                               = Server::clearAccesControlHandler;
        ac->context                             = (void*)this;
    }
    //
    // Access control
    //
    /*!
        \brief allowAddNode
        \param ac
        \param sessionId
        \param sessionContext
        \param item
        \return
    */
    virtual bool allowAddNode(UA_AccessControl* /*ac*/,
                              const UA_NodeId* /*sessionId*/,
                              void* /*sessionContext*/,
                              const UA_AddNodesItem* /*item*/)
    {
        return true;
    }

    /*!
        \brief allowAddReference
        \param ac
        \param sessionId
        \param sessionContext
        \param item
        \return
    */
    virtual bool allowAddReference(UA_AccessControl* /*ac*/,
                                   const UA_NodeId* /*sessionId*/,
                                   void* /*sessionContext*/,
                                   const UA_AddReferencesItem* /*item*/)
    {
        return true;
    }

    /*!
        \brief allowDeleteNode
        \param ac
        \param sessionId
        \param sessionContext
        \param item
        \return
    */
    virtual bool allowDeleteNode(UA_AccessControl* /*ac*/,
                                 const UA_NodeId* /*sessionId*/,
                                 void* /*sessionContext*/,
                                 const UA_DeleteNodesItem* /*item*/)
    {
        return false;  // Do not allow deletion from client
    }

    /*!
        \brief allowDeleteReference
        \param ac
        \param sessionId
        \param sessionContext
        \param item
        \return
    */
    virtual bool allowDeleteReference(UA_AccessControl* /*ac*/,
                                      const UA_NodeId* /*sessionId*/,
                                      void* /*sessionContext*/,
                                      const UA_DeleteReferencesItem* /*item*/)
    {
        return true;
    }

    /*!
        \brief activateSession
        \return
    */
    virtual UA_StatusCode activateSession(UA_AccessControl* /*ac*/,
                                          const UA_EndpointDescription* /*endpointDescription*/,
                                          const UA_ByteString* /*secureChannelRemoteCertificate*/,
                                          const UA_NodeId* /*sessionId*/,
                                          const UA_ExtensionObject* /*userIdentityToken*/,
                                          void** /*sessionContext*/)
    {
        return UA_STATUSCODE_BADSESSIONIDINVALID;
    }

    /* Deauthenticate a session and cleanup */
    virtual void closeSession(UA_AccessControl* /*ac*/, const UA_NodeId* /*sessionId*/, void* /*sessionContext*/) {}

    /* Access control for all nodes*/
    virtual uint32_t getUserRightsMask(UA_AccessControl* /*ac*/,
                                       const UA_NodeId* /*sessionId*/,
                                       void* /*sessionContext*/,
                                       const UA_NodeId* /*nodeId*/,
                                       void* /*nodeContext*/)
    {
        return 0;
    }

    /* Additional access control for variable nodes */
    virtual uint8_t getUserAccessLevel(UA_AccessControl* /*ac*/,
                                       const UA_NodeId* /*sessionId*/,
                                       void* /*sessionContext*/,
                                       const UA_NodeId* /*nodeId*/,
                                       void* /*nodeContext*/)
    {
        return 0;
    }

    /* Additional access control for method nodes */
    virtual bool getUserExecutable(UA_AccessControl* /*ac*/,
                                   const UA_NodeId* /*sessionId*/,
                                   void* /*sessionContext*/,
                                   const UA_NodeId* /*methodId*/,
                                   void* /*methodContext*/)
    {
        return false;
    }

    /*  Additional access control for calling a method node in the context of a
        specific object */
    virtual bool getUserExecutableOnObject(UA_AccessControl* ac,
                                           const UA_NodeId* sessionId,
                                           void* sessionContext,
                                           const UA_NodeId* methodId,
                                           void* methodContext,
                                           const UA_NodeId* objectId,
                                           void* objectContext)
    {
        return false;
    }
    /* Allow insert,replace,update of historical data */
    virtual bool allowHistoryUpdateUpdateData(UA_AccessControl* /*ac*/,
                                              const UA_NodeId* /*sessionId*/,
                                              void* /*sessionContext*/,
                                              const UA_NodeId* /*nodeId*/,
                                              UA_PerformUpdateType /*performInsertReplace*/,
                                              const UA_DataValue* /*value*/)
    {
        return false;
    }

    /* Allow delete of historical data */
    virtual bool allowHistoryUpdateDeleteRawModified(UA_AccessControl* /*ac*/,
                                                     const UA_NodeId* /*sessionId*/,
                                                     void* /*sessionContext*/,
                                                     const UA_NodeId* /*nodeId*/,
                                                     UA_DateTime /*startTimestamp*/,
                                                     UA_DateTime /* endTimestamp*/,
                                                     bool /*isDeleteModified*/)
    {
        return false;
    }

    /*!
     * \brief clearAccessControl
     */
    virtual void clearAccessControl(UA_AccessControl* /*ac*/)
    {
        // reset to defaults
    }
    /*!
     * \brief allowBrowseNodeHandler
     * \return
     */
    virtual bool allowBrowseNode(UA_AccessControl* /*ac*/,
                                 const UA_NodeId* /*sessionId*/,
                                 void* /*sessionContext*/,
                                 const UA_NodeId* /*nodeId*/,
                                 void* /*nodeContext*/)
    {
        return true;
    }

    /*!
     * \brief allowTransferSubscriptionHandler
     * \param ac
     * \return
     */
    virtual bool allowTransferSubscription(UA_AccessControl* ac,
                                           const UA_NodeId* /*oldSessionId*/,
                                           void* /*oldSessionContext*/,
                                           const UA_NodeId* /*newSessionId*/,
                                           void* /*newSessionContext*/)
    {
        return false;
    }

    /*!
        \brief setHistoryDatabase
    */
    void setHistoryDatabase(UA_HistoryDatabase&);
    // Publish - Subscribe interface

    // Creates an instance of a condition handler - must be derived from Condition
#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

    template <typename T>
    /*!
     * \brief createCondition
     * \param conditionType
     * \param conditionName
     * \param conditionSource
     * \param outConditionId
     * \param hierarchialReferenceType
     * \return true on success
     */
    void createCondition(const NodeId& conditionType,
                         const std::string& conditionName,
                         const NodeId& conditionSource,  // parent
                         Condition_p& outCondition,      // newly created condition object has the condition node
                         const NodeId& hierarchialReferenceType = NodeId::Null)
    {
        NodeId outConditionId;
        QualifiedName qn(conditionSource.nameSpaceIndex(), conditionName);
        outConditionId.notNull();  // this is the key to the condition dictionary
        outCondition = nullptr;
        throw_bad_status(UA_Server_createCondition(server_or_throw(),
                                                   NodeId::Null,
                                                   conditionType,
                                                   qn,
                                                   conditionSource,
                                                   hierarchialReferenceType,
                                                   outConditionId.isNull() ? nullptr : outConditionId.clearRef()));

        // create the condition object
        ConditionPtr c(new T(*this, outConditionId, conditionSource));
        outCondition       = c.get();
        unsigned key       = UA_NodeId_hash(outConditionId.clearRef());
        _conditionMap[key] = std::move(c);  // servers own the condition objects
    }

    /*!
     * \brief deleteCondition
     * \param c
     */
    void deleteCondition(const NodeId& c) { _conditionMap.erase(UA_NodeId_hash(c.ref())); }
#endif
    /*!
     * \brief setConditionTwoStateVariableCallback
     * \param condition
     * \param removeBranch
     * \param callbackType
     * \return
     */
#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
    void setConditionTwoStateVariableCallback(const NodeId& condition,
                                              UA_TwoStateVariableCallbackType callbackType,
                                              bool removeBranch = false)
    {
        ConditionPtr& c = findCondition(condition);  // conditions are bound to servers - possible for the same node id
                                                     // to be used in different servers
        if (!c)
            throw StringException("Condition not found");
        c->setCallback(callbackType, removeBranch);
    }
#endif
    /*!
     * \brief getNamespaceByName
     * \param namespaceUri
     * \param foundIndex
     * \return
     */
    void getNamespaceByName(const std::string& namespaceUri, size_t& foundIndex) const
    {
        String ua(namespaceUri);
        throw_bad_status(UA_Server_getNamespaceByName(server_or_throw(), ua, &foundIndex));
    }
    /*!
     * \brief getNamespaceByName
     * \param namespaceUri
     * \param foundIndex
     * \return
     */
    size_t getNamespaceByName(const std::string& namespaceUri) const
    {
        String ua(namespaceUri);
        size_t foundIndex;
        throw_bad_status(UA_Server_getNamespaceByName(server_or_throw(), ua, &foundIndex));
        return foundIndex;
    }

    /*!
     * \brief UA_Server_getStatistics
     * \return
     */
    UA_ServerStatistics getStatistics() { return UA_Server_getStatistics(server_or_throw()); }

    //
    // Async Access
    //
    /* Set the async flag in a method node */

    /*!
     * \brief setMethodNodeAsync
     * \param id
     * \param isAsync
     * \return
     */
    void setMethodNodeAsync(const NodeId& id, bool isAsync)
    {
        throw_bad_status(UA_Server_setMethodNodeAsync(server_or_throw(), id, (UA_Boolean)isAsync));
    }

    /*!
     * \brief getAsyncOperationNonBlocking
     * \param type
     * \param request
     * \param context
     * \param timeout
     * \return
     */
    bool getAsyncOperationNonBlocking(UA_AsyncOperationType* type,
                                      const UA_AsyncOperationRequest** request,
                                      void** context,
                                      UA_DateTime* timeout)
    {
        return UA_Server_getAsyncOperationNonBlocking(server_or_throw(), type, request, context, timeout) == UA_TRUE;
    }

    /*!
     * \brief setAsyncOperationResult
     * \param response
     * \param context
     */
    void setAsyncOperationResult(const UA_AsyncOperationResponse* response, void* context)
    {
        UA_Server_setAsyncOperationResult(server_or_throw(), response, context);
    }

    // object property

    /*!
     * \brief addTimedCallback
     * \param data
     * \param date
     * \param callbackId
     * \return
     */
    void addTimedEvent(unsigned msDelay, UA_UInt64& callbackId, std::function<void(Timer&)> func)
    {
        UA_DateTime dt = UA_DateTime_nowMonotonic() + (UA_DATETIME_MSEC * msDelay);
        TimerPtr t(new Timer(this, 0, true, func));
        throw_bad_status(
            UA_Server_addTimedCallback(server_or_throw(), Server::timerCallback, t.get(), dt, &callbackId));
        t->setId(callbackId);
        _timerMap[callbackId] = std::move(t);
    }

    /* Add a callback for cyclic repetition to the client.
     *
     * @param client The client object.
     * @param callback The callback that shall be added.
     * @param data Data that is forwarded to the callback.
     * @param interval_ms The callback shall be repeatedly executed with the given
     *        interval (in ms). The interval must be positive. The first execution
     *        occurs at now() + interval at the latest.
     * @param callbackId Set to the identifier of the repeated callback . This can
     *        be used to cancel the callback later on. If the pointer is null, the
     *        identifier is not set.
     * @return Upon success, UA_STATUSCODE_GOOD is returned. An error code
     *         otherwise. */

    void addRepeatedTimerEvent(UA_Double interval_ms, UA_UInt64& callbackId, std::function<void(Timer&)> func)
    {
        TimerPtr t(new Timer(this, 0, false, func));
        throw_bad_status(
            UA_Server_addRepeatedCallback(server_or_throw(), Server::timerCallback, t.get(), interval_ms, &callbackId));
        t->setId(callbackId);
        _timerMap[callbackId] = std::move(t);
    }
    /*!
     * \brief changeRepeatedCallbackInterval
     * \param callbackId
     * \param interval_ms
     * \return
     */
    void changeRepeatedTimerInterval(UA_UInt64 callbackId, UA_Double interval_ms)
    {
        throw_bad_status(UA_Server_changeRepeatedCallbackInterval(server_or_throw(), callbackId, interval_ms));
    }
    /*!
     * \brief UA_Client_removeCallback
     * \param client
     * \param callbackId
     */
    void removeTimerEvent(UA_UInt64 callbackId) { _timerMap.erase(callbackId); }

    //
    // Publish Subscribe Support - To be added when it is finished
    //
};

}  // namespace Open62541
#endif  // OPEN62541SERVER_H
