#include <open62541cpp/historydatabase.h>
#include <open62541cpp/open62541server.h>
/*
    Copyright (C) 2017 -  B. J. Hill

    This file is part of open62541 C++ classes. open62541 C++ classes are free software: you can
    redistribute it and/or modify it under the terms of the Mozilla Public
    License v2.0 as stated in the LICENSE file provided with open62541.

    open62541 C++ classes are distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
    A PARTICULAR PURPOSE.
*/

/*!
 * \brief Open62541::HistoryDataGathering::Context::Context
 * \param s
 * \param nId
 */
Open62541::HistoryDataGathering::Context::Context(UA_Server* s, const UA_NodeId* nId)
    : server(*Open62541::Server::findServer(s))
    , nodeId(*nId)
{
}

/*!
 * \brief Open62541::HistoryDataBackend::Context::Context
 * \param s
 * \param sId
 * \param sContext
 * \param nId
 */

Open62541::HistoryDataBackend::Context::Context(UA_Server* s,
                                                const UA_NodeId* sId,
                                                void* sContext,
                                                const UA_NodeId* nId)
    : server(*Open62541::Server::findServer(s))
    , sessionId(*sId)
    , sessionContext(sContext)
    , nodeId(*nId)
{
}

/*!
 * \brief Open62541::HistoryDatabase::Context::Context
 * \param s
 * \param sId
 * \param sContext
 * \param nId
 */
Open62541::HistoryDatabase::Context::Context(UA_Server* s, const UA_NodeId* sId, void* sContext, const UA_NodeId* nId)
    : server(*Open62541::Server::findServer(s))
    , sessionId(*sId)
    , sessionContext(sContext)
    , nodeId(*nId)
{
}

/*!
 * \brief Open62541::Historian::setUpdateNode
 * \param nodeId
 * \param server
 * \param responseSize
 * \param context
 * \return true on success
 */
bool Open62541::Historian::setUpdateNode(NodeId& nodeId,
                                         Server& server,
                                         size_t responseSize,
                                         size_t pollInterval,
                                         void* context)
{
    UA_HistorizingNodeIdSettings setting;
    setting.pollingInterval            = pollInterval;
    setting.historizingBackend         = _backend;  // set the memory database
    setting.maxHistoryDataResponseSize = responseSize;
    setting.historizingUpdateStrategy  = UA_HISTORIZINGUPDATESTRATEGY_VALUESET;
    setting.userContext                = context;
    return gathering().registerNodeId(server.server(), gathering().context, nodeId.ref(), setting) ==
           UA_STATUSCODE_GOOD;
}

/*!
 * \brief Open62541::Historian::setPollNode
 * \param nodeId
 * \param server
 * \param responseSize
 * \param pollInterval
 * \param context
 * \return true on success
 */
bool Open62541::Historian::setPollNode(NodeId& nodeId,
                                       Server& server,
                                       size_t responseSize,
                                       size_t pollInterval,
                                       void* context)
{
    UA_HistorizingNodeIdSettings setting;
    setting.historizingBackend         = _backend;  // set the memory database
    setting.pollingInterval            = pollInterval;
    setting.maxHistoryDataResponseSize = responseSize;
    setting.historizingUpdateStrategy  = UA_HISTORIZINGUPDATESTRATEGY_POLL;
    setting.userContext                = context;
    return gathering().registerNodeId(server.server(), gathering().context, nodeId.ref(), setting) ==
           UA_STATUSCODE_GOOD;
}
/*!
 * \brief Open62541::Historian::setUserNode
 * \param nodeId
 * \param server
 * \param context
 * \return true on success
 */
bool Open62541::Historian::setUserNode(NodeId& nodeId,
                                       Server& server,
                                       size_t responseSize,
                                       size_t pollInterval,
                                       void* context)
{
    UA_HistorizingNodeIdSettings setting;
    setting.historizingBackend         = _backend;  // set the memory database
    setting.pollingInterval            = pollInterval;
    setting.maxHistoryDataResponseSize = responseSize;
    setting.historizingUpdateStrategy  = UA_HISTORIZINGUPDATESTRATEGY_USER;
    setting.userContext                = context;
    return gathering().registerNodeId(server.server(), gathering().context, nodeId.ref(), setting) ==
           UA_STATUSCODE_GOOD;
}
