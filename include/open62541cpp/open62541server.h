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

namespace Open62541 {

/*!
    \brief The Server class - this abstracts the server side
*/
// This class wraps the corresponding C functions. Refer to the C documentation for a full explanation.
// The main thing to watch for is Node ID objects are passed by reference. There are stock Node Id objects including
// NodeId::Null Pass NodeId::Null where a NULL UA_NodeId pointer is expected. If a NodeId is being passed to receive a
// value use the notNull() method to mark it as a receiver of a new node id. Most functions return true if the lastError
// is UA_STATUSCODE_GOOD.

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
        virtual ~Timer() { UA_Server_removeCallback(_server->server(), _id); }
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

protected:
    UA_StatusCode _lastError = 0;

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
        if (_server) {
            _config = UA_Server_getConfig(_server);
            if (_config) {
                UA_ServerConfig_setDefault(_config);
                _config->nodeLifecycle.constructor = constructor;  // set up the node global lifecycle
                _config->nodeLifecycle.destructor  = destructor;
            }
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
        if (_server) {
            _config = UA_Server_getConfig(_server);
            if (_config) {
                UA_ServerConfig_setMinimal(_config, port, &certificate);
                _config->nodeLifecycle.constructor = constructor;  // set up the node global lifecycle
                _config->nodeLifecycle.destructor  = destructor;
            }
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
        return UA_STATUSCODE_GOOD;
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
        UA_AccessControl_default(_config, allowAnonymous, nullptr,
                                  &_config->securityPolicies[_config->securityPoliciesSize-1].policyUri,
                                 _logins.size(), _logins.data());
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
    const UA_DataType* findDataType(const NodeId& n)
    {
        if (server()) {
            return UA_Server_findDataType(server(), n.constRef());
        }
        return nullptr;
    }

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
    bool registerDiscovery(Client& client, const std::string& semaphoreFilePath = "");

    /*!
        \brief unregisterDiscovery
        \return  true on success
    */
    bool unregisterDiscovery(Client& client);

    /*!
        \brief addPeriodicServerRegister
        \param discoveryServerUrl
        \param intervalMs
        \param delayFirstRegisterMs
        \param periodicCallbackId
        \return true on success
    */
    bool addPeriodicServerRegister(const std::string& discoveryServerUrl,  // url must persist - that is be static
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
        UA_Server_setRegisterServerCallback(server(), registerServerCallback, (void*)(this));
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
        UA_Server_setServerOnNetworkCallback(server(), serverOnNetworkCallback, (void*)(this));
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
    virtual void terminate();  // called before server is closed
    //
    /*!
        \brief lastError
        \return
    */
    UA_StatusCode lastError() const { return _lastError; }

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
    bool getNodeContext(const NodeId& n, NodeContext*& c)
    {
        if (!server())
            return false;
        void* p    = (void*)(c);
        _lastError = UA_Server_getNodeContext(_server, n.get(), &p);
        return lastOK();
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
    bool setNodeContext(const NodeId& n, const NodeContext* c)
    {
        if (!server())
            return false;
        _lastError = UA_Server_setNodeContext(_server, n.get(), (void*)(c));
        return lastOK();
    }

    /*!
        \brief readAttribute
        \param nodeId
        \param attributeId
        \param v data pointer
        \return true on success
    */
    bool readAttribute(const UA_NodeId* nodeId, UA_AttributeId attributeId, void* v)
    {
        if (!server())
            return false;
        WriteLock l(_mutex);
        _lastError = __UA_Server_read(_server, nodeId, attributeId, v);
        return lastOK();
    }

    /*!
        \brief writeAttribute
        \param nodeId
        \param attributeId
        \param attr_type
        \param attr data pointer
        \return true on success
    */
    bool writeAttribute(const UA_NodeId* nodeId,
                        const UA_AttributeId attributeId,
                        const UA_DataType* attr_type,
                        const void* attr)
    {
        if (!server())
            return false;
        WriteLock l(_mutex);
        _lastError = __UA_Server_write(_server, nodeId, attributeId, attr_type, attr) == UA_STATUSCODE_GOOD;
        return lastOK();
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
    bool deleteTree(const NodeId& nodeId);
    /*!
        \brief browseTree
        \param nodeId  start point
        \param node point in tree to add nodes to
        \return true on success
    */
    bool browseTree(const UA_NodeId& nodeId, Open62541::UANode* node);  // add child nodes to property tree node

    /*!
        \brief browseTree
        \param nodeId start point to browse from
        \return true on success
    */
    bool browseTree(const NodeId& nodeId,
                    UANodeTree& tree);  // produces an addressable tree using dot seperated browse path
    /*!
        \brief browseTree
        \param nodeId start node to browse from
        \param tree tree to fill
        \return true on success
    */
    bool browseTree(const NodeId& nodeId, UANode* tree);
    /*!
        \brief browseTree
        browse and create a map of string version of nodeids ids to node ids
        \param nodeId
        \param tree
        \return true on success
    */
    bool browseTree(const NodeId& nodeId, NodeIdMap& m);  //
    /*!
        \brief browseChildren
        \param nodeId parent of childrent ot browse
        \param m map to fill
        \return true on success
    */
    bool browseChildren(const UA_NodeId& nodeId, NodeIdMap& m);

    /*  A simplified TranslateBrowsePathsToNodeIds based on the
        SimpleAttributeOperand type (Part 4, 7.4.4.5).

        This specifies a relative path using a list of BrowseNames instead of the
        RelativePath structure. The list of BrowseNames is equivalent to a
        RelativePath that specifies forward references which are subtypes of the
        HierarchicalReferences ReferenceType. All Nodes followed by the browsePath
        shall be of the NodeClass Object or Variable. */
    bool browseSimplifiedBrowsePath(const NodeId& origin,
                                    size_t browsePathSize,
                                    const QualifiedName& browsePath,
                                    BrowsePathResult& result)
    {
        result.get() = UA_Server_browseSimplifiedBrowsePath(_server, origin, browsePathSize, browsePath.constRef());
        _lastError   = result.ref()->statusCode;
        return lastOK();
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
        if (!server())
            return 0;
        UA_UInt16 ret = 0;
        {
            WriteLock l(mutex());
            ret = UA_Server_addNamespace(_server, s.c_str());
        }
        return ret;
    }
    //
    /*!
        \brief serverConfig
        \return  server configuration
    */
    UA_ServerConfig& serverConfig() { return *UA_Server_getConfig(server()); }
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
    bool addServerMethod(ServerMethod* method,
                         const std::string& browseName,
                         const NodeId& parent,
                         const NodeId& nodeId,
                         NodeId& newNode,
                         int nameSpaceIndex = 0)
    {
        //
        if (!server())
            return false;
        //
        if (nameSpaceIndex == 0)
            nameSpaceIndex = parent.nameSpaceIndex();  // inherit parent by default
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
            _lastError = UA_Server_addMethodNode(_server,
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
                                                 newNode.isNull() ? nullptr : newNode.ref());
        }
        return lastOK();
    }

    //
    //
    /*!
        \brief browseName
        \param nodeId
        \return
    */
    bool browseName(NodeId& nodeId, std::string& s, int& ns)
    {
        if (!_server)
            throw std::runtime_error("Null server");
        QualifiedName outBrowseName;
        if (UA_Server_readBrowseName(_server, nodeId, outBrowseName) == UA_STATUSCODE_GOOD) {
            s  = toString(outBrowseName.get().name);
            ns = outBrowseName.get().namespaceIndex;
        }
        return lastOK();
    }

    /*!
        \brief setBrowseName
        \param nodeId
        \param nameSpaceIndex
        \param name
    */
    void setBrowseName(const NodeId& nodeId, int nameSpaceIndex, const std::string& name)
    {
        if (!server())
            return;
        QualifiedName newBrowseName(nameSpaceIndex, name);
        WriteLock l(_mutex);
        UA_Server_writeBrowseName(_server, nodeId, newBrowseName);
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
    bool createFolderPath(const NodeId& start, const Path& path, int nameSpaceIndex, NodeId& nodeId);

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
    bool addFolder(const NodeId& parent,
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
    bool addVariable(const NodeId& parent,
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
    bool addVariable(const NodeId& parent,
                     const std::string& childName,
                     const NodeId& nodeId,
                     const std::string& c,
                     NodeId& newNode    = NodeId::Null,
                     int nameSpaceIndex = 0)
    {
        NodeContext* cp = findContext(c);
        if (cp) {
            Variant v((T()));
            return addVariable(parent, childName, v, nodeId, newNode, cp, nameSpaceIndex);
        }
        return false;
    }

    /*!
        \brief addVariable
        \param parent
        \param nameSpaceIndex
        \param childName
        \return true on success
    */
    bool addHistoricalVariable(const NodeId& parent,
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
    bool addHistoricalVariable(const NodeId& parent,
                               const std::string& childName,
                               const NodeId& nodeId,
                               const std::string& c,
                               NodeId& newNode    = NodeId::Null,
                               int nameSpaceIndex = 0)
    {
        NodeContext* cp = findContext(c);
        if (cp) {
            Variant v((T()));
            return addHistoricalVariable(parent, childName, v, nodeId, newNode, cp, nameSpaceIndex);
        }
        return false;
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
    bool addProperty(const NodeId& parent,
                     const std::string& key,
                     const T& value,
                     const NodeId& nodeId = NodeId::Null,
                     NodeId& newNode      = NodeId::Null,
                     NodeContext* c       = nullptr,
                     int nameSpaceIndex   = 0)
    {
        Variant v(value);
        return addProperty(parent, key, v, nodeId, newNode, c, nameSpaceIndex);
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
    bool addProperty(const NodeId& parent,
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
    bool variable(const NodeId& nodeId, Variant& value)
    {
        if (!server())
            return false;

        // outValue is managed by caller - transfer to output value
        value.null();
        WriteLock l(_mutex);
        UA_Server_readValue(_server, nodeId, value.ref());
        return lastOK();
    }
    /*!
        \brief deleteNode
        \param nodeId
        \param deleteReferences
        \return true on success
    */
    bool deleteNode(const NodeId& nodeId, bool deleteReferences)
    {
        if (!server())
            return false;

        WriteLock l(_mutex);
        _lastError = UA_Server_deleteNode(_server, nodeId, UA_Boolean(deleteReferences));
        return _lastError != UA_STATUSCODE_GOOD;
    }

    /*!
        \brief call
        \param request
        \param ret
        \return true on sucess
    */
    bool call(const CallMethodRequest& request, CallMethodResult& ret)
    {
        if (!server())
            return false;

        WriteLock l(_mutex);
        ret.get() = UA_Server_call(_server, request);
        return ret.get().statusCode == UA_STATUSCODE_GOOD;
    }

    /*!
        \brief translateBrowsePathToNodeIds
        \param path
        \param result
        \return true on sucess
    */
    bool translateBrowsePathToNodeIds(const BrowsePath& path, BrowsePathResult& result)
    {
        if (!server())
            return false;

        WriteLock l(_mutex);
        result.get() = UA_Server_translateBrowsePathToNodeIds(_server, path);
        return result.get().statusCode == UA_STATUSCODE_GOOD;
    }

    /*!
        \brief lastOK
        \return last error code
    */
    bool lastOK() const { return _lastError == UA_STATUSCODE_GOOD; }
    //
    // Attributes
    //
    /*!
        \brief readNodeId
        \param nodeId
        \param outNodeId
        \return true on sucess
    */
    bool readNodeId(const NodeId& nodeId, NodeId& outNodeId)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_NODEID, outNodeId);
    }
    /*!
        \brief readNodeClass
        \param nodeId
        \param outNodeClass
        \return true on success
    */
    bool readNodeClass(const NodeId& nodeId, UA_NodeClass& outNodeClass)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_NODECLASS, &outNodeClass);
    }
    /*!
        \brief readBrowseName
        \param nodeId
        \param outBrowseName
        \return true on success
    */
    bool readBrowseName(const NodeId& nodeId, QualifiedName& outBrowseName)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_BROWSENAME, outBrowseName);
    }
    /*!
        \brief readDisplayName
        \param nodeId
        \param outDisplayName
        \return true on sucess
    */
    bool readDisplayName(const NodeId& nodeId, LocalizedText& outDisplayName)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_DISPLAYNAME, outDisplayName);
    }
    /*!
        \brief readDescription
        \param nodeId
        \param outDescription
        \return true on success
    */
    bool readDescription(const NodeId& nodeId, LocalizedText& outDescription)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_DESCRIPTION, outDescription);
    }
    /*!
        \brief readWriteMask
        \param nodeId
        \param outWriteMask
        \return true on sucess
    */
    bool readWriteMask(const NodeId& nodeId, UA_UInt32& outWriteMask)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_WRITEMASK, &outWriteMask);
    }
    /*!
        \brief readIsAbstract
        \param nodeId
        \param outIsAbstract
        \return true on success
    */
    bool readIsAbstract(const NodeId& nodeId, UA_Boolean& outIsAbstract)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_ISABSTRACT, &outIsAbstract);
    }
    /*!
        \brief readSymmetric
        \param nodeId
        \param outSymmetric
        \return true on success
    */
    bool readSymmetric(const NodeId& nodeId, UA_Boolean& outSymmetric)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_SYMMETRIC, &outSymmetric);
    }
    /*!
        \brief readInverseName
        \param nodeId
        \param outInverseName
        \return true on success
    */
    bool readInverseName(const NodeId& nodeId, LocalizedText& outInverseName)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_INVERSENAME, outInverseName);
    }
    /*!
        \brief readContainsNoLoop
        \param nodeId
        \param outContainsNoLoops
        \return true on success
    */
    bool readContainsNoLoop(const NodeId& nodeId, UA_Boolean& outContainsNoLoops)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_CONTAINSNOLOOPS, &outContainsNoLoops);
    }
    /*!
        \brief readEventNotifier
        \param nodeId
        \param outEventNotifier
        \return
    */
    bool readEventNotifier(const NodeId& nodeId, UA_Byte& outEventNotifier)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_EVENTNOTIFIER, &outEventNotifier);
    }
    /*!
        \brief readValue
        \param nodeId
        \param outValue
        \return
    */
    bool readValue(const NodeId& nodeId, Variant& outValue)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_VALUE, outValue);
    }
    /*!
        \brief readDataType
        \param nodeId
        \param outDataType
        \return
    */
    bool readDataType(const NodeId& nodeId, NodeId& outDataType)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_DATATYPE, outDataType);
    }
    /*!
        \brief readValueRank
        \param nodeId
        \param outValueRank
        \return
    */
    bool readValueRank(const NodeId& nodeId, UA_Int32& outValueRank)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_VALUERANK, &outValueRank);
    }

    /* Returns a variant with an int32 array */
    /*!
        \brief readArrayDimensions
        \param nodeId
        \param outArrayDimensions
        \return
    */
    bool readArrayDimensions(const NodeId& nodeId, Variant& outArrayDimensions)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_ARRAYDIMENSIONS, outArrayDimensions);
    }
    /*!
        \brief readAccessLevel
        \param nodeId
        \param outAccessLevel
        \return
    */
    bool readAccessLevel(const NodeId& nodeId, UA_Byte& outAccessLevel)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_ACCESSLEVEL, &outAccessLevel);
    }
    /*!
        \brief readMinimumSamplingInterval
        \param nodeId
        \param outMinimumSamplingInterval
        \return
    */
    bool readMinimumSamplingInterval(const NodeId& nodeId, UA_Double& outMinimumSamplingInterval)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_MINIMUMSAMPLINGINTERVAL, &outMinimumSamplingInterval);
    }
    /*!
        \brief readHistorizing
        \param nodeId
        \param outHistorizing
        \return
    */
    bool readHistorizing(const NodeId& nodeId, UA_Boolean& outHistorizing)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_HISTORIZING, &outHistorizing);
    }
    /*!
        \brief readExecutable
        \param nodeId
        \param outExecutable
        \return
    */
    bool readExecutable(const NodeId& nodeId, UA_Boolean& outExecutable)
    {
        return readAttribute(nodeId, UA_ATTRIBUTEID_EXECUTABLE, &outExecutable);
    }

    bool readObjectProperty(const NodeId& objectId, const QualifiedName& propertyName, Variant& value)
    {
        return UA_Server_readObjectProperty(server(), objectId, propertyName, value) == UA_STATUSCODE_GOOD;
    }
    /*!
        \brief writeBrowseName
        \param nodeId
        \param browseName
        \return
    */
    bool writeBrowseName(const NodeId& nodeId, QualifiedName& browseName)
    {
        return writeAttribute(nodeId, UA_ATTRIBUTEID_BROWSENAME, &UA_TYPES[UA_TYPES_QUALIFIEDNAME], browseName);
    }
    /*!
        \brief writeDisplayName
        \param nodeId
        \param displayName
        \return
    */
    bool writeDisplayName(const NodeId& nodeId, LocalizedText& displayName)
    {
        return writeAttribute(nodeId, UA_ATTRIBUTEID_DISPLAYNAME, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT], displayName);
    }
    /*!
        \brief writeDescription
        \param nodeId
        \param description
        \return
    */
    bool writeDescription(const NodeId& nodeId, LocalizedText& description)
    {
        return writeAttribute(nodeId, UA_ATTRIBUTEID_DESCRIPTION, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT], description);
    }
    /*!
        \brief writeWriteMask
        \param nodeId
        \param writeMask
        \return
    */
    bool writeWriteMask(const NodeId& nodeId, const UA_UInt32 writeMask)
    {
        return writeAttribute(nodeId, UA_ATTRIBUTEID_WRITEMASK, &UA_TYPES[UA_TYPES_UINT32], &writeMask);
    }
    /*!
        \brief writeIsAbstract
        \param nodeId
        \param isAbstract
        \return
    */
    bool writeIsAbstract(const NodeId& nodeId, const UA_Boolean isAbstract)
    {
        return writeAttribute(nodeId, UA_ATTRIBUTEID_ISABSTRACT, &UA_TYPES[UA_TYPES_BOOLEAN], &isAbstract);
    }
    /*!
        \brief writeInverseName
        \param nodeId
        \param inverseName
        \return
    */
    bool writeInverseName(const NodeId& nodeId, const UA_LocalizedText inverseName)
    {
        return writeAttribute(nodeId, UA_ATTRIBUTEID_INVERSENAME, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT], &inverseName);
    }
    /*!
        \brief writeEventNotifier
        \param nodeId
        \param eventNotifier
        \return
    */
    bool writeEventNotifier(const NodeId& nodeId, const UA_Byte eventNotifier)
    {
        return writeAttribute(nodeId, UA_ATTRIBUTEID_EVENTNOTIFIER, &UA_TYPES[UA_TYPES_BYTE], &eventNotifier);
    }
    /*!
        \brief writeValue
        \param nodeId
        \param value
        \return
    */
    bool writeValue(const NodeId& nodeId, const Variant& value)
    {
        if (!server())
            return false;

        return UA_STATUSCODE_GOOD ==
               (_lastError =
                    __UA_Server_write(_server, nodeId, UA_ATTRIBUTEID_VALUE, &UA_TYPES[UA_TYPES_VARIANT], value));
    }
    /*!
        \brief writeDataType
        \param nodeId
        \param dataType
        \return
    */
    bool writeDataType(const NodeId& nodeId, const NodeId& dataType)
    {
        return writeAttribute(nodeId, UA_ATTRIBUTEID_DATATYPE, &UA_TYPES[UA_TYPES_NODEID], dataType);
    }
    /*!
        \brief writeValueRank
        \param nodeId
        \param valueRank
        \return
    */
    bool writeValueRank(const NodeId& nodeId, const UA_Int32 valueRank)
    {
        return writeAttribute(nodeId, UA_ATTRIBUTEID_VALUERANK, &UA_TYPES[UA_TYPES_INT32], &valueRank);
    }

    /*!
        \brief writeArrayDimensions
        \param nodeId
        \param arrayDimensions
        \return
    */
    bool writeArrayDimensions(const NodeId& nodeId, const Variant arrayDimensions)
    {
        return writeAttribute(nodeId, UA_ATTRIBUTEID_VALUE, &UA_TYPES[UA_TYPES_VARIANT], arrayDimensions.constRef());
    }
    /*!
        \brief writeAccessLevel
        \param nodeId
        \param accessLevel
        \return
    */
    bool writeAccessLevel(const NodeId& nodeId, const UA_Byte accessLevel)
    {
        return writeAttribute(nodeId, UA_ATTRIBUTEID_ACCESSLEVEL, &UA_TYPES[UA_TYPES_BYTE], &accessLevel);
    }

    // Some short cuts
    /*!
        \brief writeEnable
        \param nodeId
        \return
    */
    bool writeEnable(const NodeId& nodeId)
    {
        UA_Byte accessLevel;
        if (readAccessLevel(nodeId, accessLevel)) {
            accessLevel |= UA_ACCESSLEVELMASK_WRITE;
            return writeAccessLevel(nodeId, accessLevel);
        }
        return false;
    }
    /*!
        \brief setReadOnly
        \param nodeId
        \param historyEnable
        \return
    */
    bool setReadOnly(const NodeId& nodeId, bool historyEnable = false)
    {
        UA_Byte accessLevel;
        if (readAccessLevel(nodeId, accessLevel)) {
            // remove the write bits
            accessLevel &= ~(UA_ACCESSLEVELMASK_WRITE | UA_ACCESSLEVELMASK_HISTORYWRITE);
            // add the read bits
            accessLevel |= UA_ACCESSLEVELMASK_READ;
            if (historyEnable)
                accessLevel |= UA_ACCESSLEVELMASK_HISTORYREAD;
            return writeAccessLevel(nodeId, accessLevel);
        }
        return false;
    }

    /*!
        \brief writeMinimumSamplingInterval
        \param nodeId
        \param miniumSamplingInterval
        \return
    */
    bool writeMinimumSamplingInterval(const NodeId& nodeId, const UA_Double miniumSamplingInterval)
    {
        return writeAttribute(nodeId,
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
    bool writeExecutable(const NodeId& nodeId, const UA_Boolean executable)
    {
        return writeAttribute(nodeId, UA_ATTRIBUTEID_EXECUTABLE, &UA_TYPES[UA_TYPES_BOOLEAN], &executable);
    }

    /*!
     * \brief writeObjectProperty
     * \param objectId
     * \param propertyName
     * \param value
     * \return true on success
     */
    bool writeObjectProperty(const NodeId& objectId, const QualifiedName& propertyName, const Variant& value)
    {
        return UA_Server_writeObjectProperty(server(), objectId, propertyName, value) == UA_STATUSCODE_GOOD;
    }

    template <typename T>
    /*!
     * \brief writeObjectProperty
     * \param objectId
     * \param propertyName
     * \param value
     * \return
     */
    bool writeObjectProperty(const NodeId& objectId, const std::string& propertyName, const T& value)
    {
        Variant v(value);
        QualifiedName qn(0, propertyName);
        return writeObjectProperty(objectId, qn, v);
    }
    /*!
     * \brief writeObjectProperty_scalar
     * \param objectId
     * \param propertyName
     * \param value
     * \param type
     * \return true on success
     */
    bool writeObjectProperty_scalar(const NodeId& objectId,
                                    const std::string& propertyName,
                                    const void* value,
                                    const UA_DataType* type)
    {
        QualifiedName qn(0, propertyName);
        return UA_Server_writeObjectProperty_scalar(server(), objectId, qn, value, type) == UA_STATUSCODE_GOOD;
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
    bool addVariableNode(const NodeId& requestedNewNodeId,
                         const NodeId& parentNodeId,
                         const NodeId& referenceTypeId,
                         const QualifiedName& browseName,
                         const NodeId& typeDefinition,
                         const VariableAttributes& attr,
                         NodeId& outNewNodeId = NodeId::Null,
                         NodeContext* nc      = nullptr)
    {
        if (!server())
            return false;
        WriteLock l(_mutex);
        _lastError = UA_Server_addVariableNode(_server,
                                               requestedNewNodeId,
                                               parentNodeId,
                                               referenceTypeId,
                                               browseName,
                                               typeDefinition,
                                               attr,
                                               nc,
                                               outNewNodeId.isNull() ? nullptr : outNewNodeId.ref());

        return lastOK();
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
    bool addVariableTypeNode(const NodeId& requestedNewNodeId,
                             const NodeId& parentNodeId,
                             const NodeId& referenceTypeId,
                             const QualifiedName& browseName,
                             const NodeId& typeDefinition,
                             const VariableTypeAttributes& attr,
                             NodeId& outNewNodeId               = NodeId::Null,
                             NodeContext* instantiationCallback = nullptr)
    {
        if (!server())
            return false;
        WriteLock l(_mutex);
        _lastError = UA_Server_addVariableTypeNode(_server,
                                                   requestedNewNodeId,
                                                   parentNodeId,
                                                   referenceTypeId,
                                                   browseName,
                                                   typeDefinition,
                                                   attr,
                                                   instantiationCallback,
                                                   outNewNodeId.isNull() ? nullptr : outNewNodeId.ref());
        return lastOK();
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
    bool addObjectNode(const NodeId& requestedNewNodeId,
                       const NodeId& parentNodeId,
                       const NodeId& referenceTypeId,
                       const QualifiedName& browseName,
                       const NodeId& typeDefinition,
                       const ObjectAttributes& attr,
                       NodeId& outNewNodeId               = NodeId::Null,
                       NodeContext* instantiationCallback = nullptr)
    {
        if (!server())
            return false;

        WriteLock l(_mutex);
        _lastError = UA_Server_addObjectNode(_server,
                                             requestedNewNodeId,
                                             parentNodeId,
                                             referenceTypeId,
                                             browseName,
                                             typeDefinition,
                                             attr,
                                             instantiationCallback,
                                             outNewNodeId.isNull() ? nullptr : outNewNodeId.clearRef());
        return lastOK();
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
    bool addObjectTypeNode(const NodeId& requestedNewNodeId,
                           const NodeId& parentNodeId,
                           const NodeId& referenceTypeId,
                           const QualifiedName& browseName,
                           const ObjectTypeAttributes& attr,
                           NodeId& outNewNodeId               = NodeId::Null,
                           NodeContext* instantiationCallback = nullptr)
    {
        if (!server())
            return false;

        _lastError = UA_Server_addObjectTypeNode(_server,
                                                 requestedNewNodeId,
                                                 parentNodeId,
                                                 referenceTypeId,
                                                 browseName,
                                                 attr,
                                                 instantiationCallback,
                                                 outNewNodeId.isNull() ? nullptr : outNewNodeId.clearRef());
        return lastOK();
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
    bool addViewNode(const NodeId& requestedNewNodeId,
                     const NodeId& parentNodeId,
                     const NodeId& referenceTypeId,
                     const QualifiedName& browseName,
                     const ViewAttributes& attr,
                     NodeId& outNewNodeId               = NodeId::Null,
                     NodeContext* instantiationCallback = nullptr)
    {
        if (!server())
            return false;

        WriteLock l(_mutex);
        _lastError = UA_Server_addViewNode(_server,
                                           requestedNewNodeId,
                                           parentNodeId,
                                           referenceTypeId,
                                           browseName,
                                           attr,
                                           instantiationCallback,
                                           outNewNodeId.isNull() ? nullptr : outNewNodeId.clearRef());
        return lastOK();
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
    bool addReferenceTypeNode(const NodeId& requestedNewNodeId,
                              const NodeId& parentNodeId,
                              const NodeId& referenceTypeId,
                              const QualifiedName& browseName,
                              const ReferenceTypeAttributes& attr,
                              NodeId& outNewNodeId               = NodeId::Null,
                              NodeContext* instantiationCallback = nullptr)
    {
        if (!server())
            return false;

        WriteLock l(_mutex);
        _lastError = UA_Server_addReferenceTypeNode(_server,
                                                    requestedNewNodeId,
                                                    parentNodeId,
                                                    referenceTypeId,
                                                    browseName,
                                                    attr,
                                                    instantiationCallback,
                                                    outNewNodeId.isNull() ? nullptr : outNewNodeId.clearRef());
        return lastOK();
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
    bool addDataTypeNode(const NodeId& requestedNewNodeId,
                         const NodeId& parentNodeId,
                         const NodeId& referenceTypeId,
                         const QualifiedName& browseName,
                         const DataTypeAttributes& attr,
                         NodeId& outNewNodeId               = NodeId::Null,
                         NodeContext* instantiationCallback = nullptr)
    {
        if (!server())
            return false;

        WriteLock l(_mutex);
        _lastError = UA_Server_addDataTypeNode(_server,
                                               requestedNewNodeId,
                                               parentNodeId,
                                               referenceTypeId,
                                               browseName,
                                               attr,
                                               instantiationCallback,
                                               outNewNodeId.isNull() ? nullptr : outNewNodeId.clearRef());
        return lastOK();
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
    bool addDataSourceVariableNode(const NodeId& requestedNewNodeId,
                                   const NodeId& parentNodeId,
                                   const NodeId& referenceTypeId,
                                   const QualifiedName& browseName,
                                   const NodeId& typeDefinition,
                                   const VariableAttributes& attr,
                                   const DataSource& dataSource,
                                   NodeId& outNewNodeId               = NodeId::Null,
                                   NodeContext* instantiationCallback = nullptr)
    {
        if (!server())
            return false;

        WriteLock l(_mutex);
        _lastError = UA_Server_addDataSourceVariableNode(_server,
                                                         requestedNewNodeId,
                                                         parentNodeId,
                                                         referenceTypeId,
                                                         browseName,
                                                         typeDefinition,
                                                         attr,
                                                         dataSource,
                                                         instantiationCallback,
                                                         outNewNodeId.isNull() ? nullptr : outNewNodeId.ref());

        return lastOK();
    }

    /*!
        \brief addReference
        \param sourceId
        \param refTypeId
        \param targetId
        \param isForward
        \return
    */
    bool addReference(const NodeId& sourceId, const NodeId& refTypeId, const ExpandedNodeId& targetId, bool isForward)
    {
        if (!server())
            return false;
        WriteLock l(_mutex);
        _lastError = UA_Server_addReference(server(), sourceId, refTypeId, targetId, isForward);
        return lastOK();
    }

    /*!
        \brief markMandatory
        \param nodeId
        \return
    */
    bool markMandatory(const NodeId& nodeId)
    {
        return addReference(nodeId, NodeId::HasModellingRule, ExpandedNodeId::ModellingRuleMandatory, true);
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
    bool deleteReference(const NodeId& sourceNodeId,
                         const NodeId& referenceTypeId,
                         bool isForward,
                         const ExpandedNodeId& targetNodeId,
                         bool deleteBidirectional)
    {
        if (!server())
            return false;

        WriteLock l(_mutex);
        _lastError = UA_Server_deleteReference(server(),
                                               sourceNodeId,
                                               referenceTypeId,
                                               isForward,
                                               targetNodeId,
                                               deleteBidirectional);
        return lastOK();
    }

    /*!
        \brief Open62541::Server::addInstance
        \param n
        \param parent
        \param nodeId
        \return
    */
    bool addInstance(const std::string& n,
                     const NodeId& requestedNewNodeId,
                     const NodeId& parent,
                     const NodeId& typeId,
                     NodeId& nodeId       = NodeId::Null,
                     NodeContext* context = nullptr)
    {
        if (!server())
            return false;

        ObjectAttributes oAttr;
        oAttr.setDefault();
        oAttr.setDisplayName(n);
        QualifiedName qn(parent.nameSpaceIndex(), n);
        return addObjectNode(requestedNewNodeId, parent, NodeId::Organizes, qn, typeId, oAttr, nodeId, context);
    }
    //
    //
    //
    /*  Creates a node representation of an event

        @param server The server object
        @param eventType The type of the event for which a node should be created
        @param outNodeId The NodeId of the newly created node for the event
        @return The StatusCode of the UA_Server_createEvent method */
    bool createEvent(const NodeId& eventType, NodeId& outNodeId)
    {
        if (!server())
            return false;
        WriteLock l(_mutex);
        _lastError = UA_Server_createEvent(_server, eventType, outNodeId.ref());
        return lastOK();
    }

    /*  Triggers a node representation of an event by applying EventFilters and
        adding the event to the appropriate queues.
        @param server The server object
        @param eventNodeId The NodeId of the node representation of the event which should be triggered
        @param outEvent the EventId of the new event
        @param deleteEventNode Specifies whether the node representation of the event should be deleted
        @return The StatusCode of the UA_Server_triggerEvent method */
    bool triggerEvent(const NodeId& eventNodeId,
                      const NodeId& sourceNode,
                      UA_ByteString* outEventId = nullptr,
                      bool deleteEventNode      = true)
    {
        if (!server())
            return false;
        WriteLock l(_mutex);
        _lastError = UA_Server_triggerEvent(_server, eventNodeId, sourceNode, outEventId, deleteEventNode);
        return lastOK();
    }

    /*!
     \brief addNewEventType
     \param name
     \param description
     \param eventType  the event type node
     \return true on success
 */
    bool addNewEventType(const std::string& name,
                         NodeId& eventType,
                         const std::string& description = std::string())
    {
        if (!server())
            return false;
        ObjectTypeAttributes attr;
        attr.setDefault();
        attr.setDisplayName(name);
        attr.setDescription((description.empty() ? name : description));
        QualifiedName qn(0, name);
        WriteLock l(_mutex);
        _lastError = UA_Server_addObjectTypeNode(server(),
                                                 UA_NODEID_NULL,
                                                 UA_NODEID_NUMERIC(0, UA_NS0ID_BASEEVENTTYPE),
                                                 UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
                                                 qn,
                                                 attr,
                                                 NULL,
                                                 eventType.clearRef());
        return lastOK();
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

    bool setUpEvent(NodeId& outId,
                    const NodeId& eventType,
                    const std::string& eventMessage,
                    const std::string& eventSourceName,
                    int eventSeverity     = 100,
                    UA_DateTime eventTime = UA_DateTime_now())
    {
        if (!server())
            return false;
        WriteLock l(_mutex);
        outId.notNull();
        _lastError = UA_Server_createEvent(server(), eventType, outId);
        if (lastOK()) {

            /* Set the Event Attributes */
            /* Setting the Time is required or else the event will not show up in UAExpert! */
            UA_Server_writeObjectProperty_scalar(server(),
                                                 outId,
                                                 UA_QUALIFIEDNAME(0, const_cast<char*>("Time")),
                                                 &eventTime,
                                                 &UA_TYPES[UA_TYPES_DATETIME]);

            UA_Server_writeObjectProperty_scalar(server(),
                                                 outId,
                                                 UA_QUALIFIEDNAME(0, const_cast<char*>("Severity")),
                                                 &eventSeverity,
                                                 &UA_TYPES[UA_TYPES_UINT16]);

            LocalizedText eM(const_cast<char*>("en-US"), eventMessage);
            UA_Server_writeObjectProperty_scalar(server(),
                                                 outId,
                                                 UA_QUALIFIEDNAME(0, const_cast<char*>("Message")),
                                                 eM.clearRef(),
                                                 &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);

            UA_String eSN = UA_STRING(const_cast<char*>(eventSourceName.c_str()));
            UA_Server_writeObjectProperty_scalar(server(),
                                                 outId,
                                                 UA_QUALIFIEDNAME(0, const_cast<char*>("SourceName")),
                                                 &eSN,
                                                 &UA_TYPES[UA_TYPES_STRING]);
        }
        return lastOK();
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
    bool updateCertificate(const UA_ByteString* oldCertificate,
                           const UA_ByteString* newCertificate,
                           const UA_ByteString* newPrivateKey,
                           bool closeSessions       = true,
                           bool closeSecureChannels = true)
    {
        if (!server())
            return false;
        WriteLock l(_mutex);
        _lastError = UA_Server_updateCertificate(_server,
                                                 oldCertificate,
                                                 newCertificate,
                                                 newPrivateKey,
                                                 closeSessions,
                                                 closeSecureChannels);
        return lastOK();
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
    bool createCondition(const NodeId& conditionType,
                         const std::string& conditionName,
                         const NodeId& conditionSource,  // parent
                         Condition_p& outCondition,      // newly created condition object has the condition node
                         const NodeId& hierarchialReferenceType = NodeId::Null)
    {
        NodeId outConditionId;
        QualifiedName qn(conditionSource.nameSpaceIndex(), conditionName);
        outConditionId.notNull();  // this is the key to the condition dictionary
        outCondition = nullptr;
        _lastError   = UA_Server_createCondition(server(),
                                               NodeId::Null,
                                               conditionType,
                                               qn,
                                               conditionSource,
                                               hierarchialReferenceType,
                                               outConditionId.isNull() ? nullptr : outConditionId.clearRef());
        if (lastOK()) {
            // create the condition object
            ConditionPtr c(new T(*this, outConditionId, conditionSource));
            outCondition       = c.get();
            unsigned key       = UA_NodeId_hash(outConditionId.clearRef());
            _conditionMap[key] = std::move(c);  // servers own the condition objects
            return true;
        }
        return false;
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
    bool setConditionTwoStateVariableCallback(const NodeId& condition,
                                              UA_TwoStateVariableCallbackType callbackType,
                                              bool removeBranch = false)
    {
        ConditionPtr& c = findCondition(condition);  // conditions are bound to servers - possible for the same node id
                                                     // to be used in different servers
        if (c) {
            return c->setCallback(callbackType, removeBranch);
        }
        return false;
    }
#endif
    /*!
     * \brief getNamespaceByName
     * \param namespaceUri
     * \param foundIndex
     * \return
     */
    bool getNamespaceByName(const std::string& namespaceUri, size_t& foundIndex)
    {
        String ua(namespaceUri);
        _lastError = UA_Server_getNamespaceByName(server(), ua, &foundIndex);
        return lastOK();
    }

    /*!
     * \brief UA_Server_getStatistics
     * \return
     */
    UA_ServerStatistics getStatistics() { return UA_Server_getStatistics(server()); }

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
    bool setMethodNodeAsync(const NodeId& id, bool isAsync)
    {

        _lastError = UA_Server_setMethodNodeAsync(server(), id, (UA_Boolean)isAsync);
        return lastOK();
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
        return UA_Server_getAsyncOperationNonBlocking(server(), type, request, context, timeout) == UA_TRUE;
    }

    /*!
     * \brief setAsyncOperationResult
     * \param response
     * \param context
     */
    void setAsyncOperationResult(const UA_AsyncOperationResponse* response, void* context)
    {
        UA_Server_setAsyncOperationResult(server(), response, context);
    }

    // object property

    /*!
     * \brief addTimedCallback
     * \param data
     * \param date
     * \param callbackId
     * \return
     */
    bool addTimedEvent(unsigned msDelay, UA_UInt64& callbackId, std::function<void(Timer&)> func)
    {
        if (_server) {
            UA_DateTime dt = UA_DateTime_nowMonotonic() + (UA_DATETIME_MSEC * msDelay);
            TimerPtr t(new Timer(this, 0, true, func));
            _lastError = UA_Server_addTimedCallback(_server, Server::timerCallback, t.get(), dt, &callbackId);
            t->setId(callbackId);
            _timerMap[callbackId] = std::move(t);
            return lastOK();
        }
        callbackId = 0;
        return false;
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

    bool addRepeatedTimerEvent(UA_Double interval_ms, UA_UInt64& callbackId, std::function<void(Timer&)> func)
    {
        if (_server) {
            TimerPtr t(new Timer(this, 0, false, func));
            _lastError =
                UA_Server_addRepeatedCallback(_server, Server::timerCallback, t.get(), interval_ms, &callbackId);
            t->setId(callbackId);
            _timerMap[callbackId] = std::move(t);
            return lastOK();
        }
        callbackId = 0;
        return false;
    }
    /*!
     * \brief changeRepeatedCallbackInterval
     * \param callbackId
     * \param interval_ms
     * \return
     */
    bool changeRepeatedTimerInterval(UA_UInt64 callbackId, UA_Double interval_ms)
    {
        if (_server) {
            _lastError = UA_Server_changeRepeatedCallbackInterval(_server, callbackId, interval_ms);
            return lastOK();
        }
        return false;
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
