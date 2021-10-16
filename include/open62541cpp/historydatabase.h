#ifndef HISTORYDATABASE_H
#define HISTORYDATABASE_H
/*
    Copyright (C) 2017 -  B. J. Hill

    This file is part of open62541 C++ classes. open62541 C++ classes are free software: you can
    redistribute it and/or modify it under the terms of the Mozilla Public
    License v2.0 as stated in the LICENSE file provided with open62541.

    open62541 C++ classes are distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
    A PARTICULAR PURPOSE.
*/

#include <open62541cpp/open62541objects.h>
namespace Open62541 {

class Server;

// Wrap the Historian classes in C++
// probably the memory database will be all that is needed most of the time
//

/*!
    \brief The HistoryDataGathering class
*/
class HistoryDataGathering
{

public:
    // wrap the standard arg items into a single struct to make life easier
    struct Context {
        Server& server;
        NodeId sessionId;
        void* sessionContext = nullptr;
        NodeId nodeId;
        Context(UA_Server* s, const UA_NodeId* nId = nullptr);
    };

private:
    //
    UA_HistoryDataGathering _gathering;
    // Static callbacks
    //
    static void _deleteMembers(UA_HistoryDataGathering* gathering)
    {
        if (gathering && gathering->context) {
            HistoryDataGathering* p = static_cast<HistoryDataGathering*>(gathering->context);
            p->deleteMembers();
        }
    }

    /*  This function registers a node for the gathering of historical data.

        server is the server the node lives in.
        hdgContext is the context of the UA_HistoryDataGathering.
        nodeId is the node id of the node to register.
        setting contains the gatering settings for the node to register. */
    static UA_StatusCode _registerNodeId(UA_Server* server,
                                         void* hdgContext,
                                         const UA_NodeId* nodeId,
                                         const UA_HistorizingNodeIdSettings setting)
    {
        if (hdgContext) {
            Context c(server, nodeId);
            HistoryDataGathering* p = static_cast<HistoryDataGathering*>(hdgContext);
            return p->registerNodeId(c, setting);
        }
        return 0;
    }

    /*  This function stops polling a node for value changes.

        server is the server the node lives in.
        hdgContext is the context of the UA_HistoryDataGathering.
        nodeId is id of the node for which polling shall be stopped.
        setting contains the gatering settings for the node. */
    static UA_StatusCode _stopPoll(UA_Server* server, void* hdgContext, const UA_NodeId* nodeId)
    {
        if (hdgContext) {
            Context c(server, nodeId);
            HistoryDataGathering* p = static_cast<HistoryDataGathering*>(hdgContext);
            return p->stopPoll(c);
        }
        return 0;
    }

    /*  This function starts polling a node for value changes.

        server is the server the node lives in.
        hdgContext is the context of the UA_HistoryDataGathering.
        nodeId is the id of the node for which polling shall be started. */
    static UA_StatusCode _startPoll(UA_Server* server, void* hdgContext, const UA_NodeId* nodeId)
    {
        if (hdgContext) {
            Context c(server, nodeId);
            HistoryDataGathering* p = static_cast<HistoryDataGathering*>(hdgContext);
            return p->startPoll(c);
        }
        return 0;
    }

    /*  This function modifies the gathering settings for a node.

        server is the server the node lives in.
        hdgContext is the context of the UA_HistoryDataGathering.
        nodeId is the node id of the node for which gathering shall be modified.
        setting contains the new gatering settings for the node. */
    static UA_Boolean _updateNodeIdSetting(UA_Server* server,
                                           void* hdgContext,
                                           const UA_NodeId* nodeId,
                                           const UA_HistorizingNodeIdSettings setting)
    {
        if (hdgContext) {
            Context c(server, nodeId);
            HistoryDataGathering* p = static_cast<HistoryDataGathering*>(hdgContext);
            return p->updateNodeIdSetting(c, setting);
        }
        return 0;
    }

    /*  Returns the gathering settings for a node.

        server is the server the node lives in.
        hdgContext is the context of the UA_HistoryDataGathering.
        nodeId is the node id of the node for which the gathering settings shall
              be retrieved. */
    static const UA_HistorizingNodeIdSettings* _getHistorizingSetting(UA_Server* server,
                                                                      void* hdgContext,
                                                                      const UA_NodeId* nodeId)
    {
        if (hdgContext) {
            Context c(server, nodeId);
            HistoryDataGathering* p = static_cast<HistoryDataGathering*>(hdgContext);
            return p->getHistorizingSetting(c);
        }
        return 0;
    }

    /*  Sets a DataValue for a node in the historical data storage.

        server is the server the node lives in.
        hdgContext is the context of the UA_HistoryDataGathering.
        sessionId and sessionContext identify the session which wants to set this value.
        nodeId is the node id of the node for which a value shall be set.
        historizing is the historizing flag of the node identified by nodeId.
        value is the value to set in the history data storage. */
    static void _setValue(UA_Server* server,
                          void* hdgContext,
                          const UA_NodeId* sessionId,
                          void* sessionContext,
                          const UA_NodeId* nodeId,
                          UA_Boolean historizing,
                          const UA_DataValue* value)
    {
        if (hdgContext) {
            Context c(server, nodeId);
            c.sessionContext = sessionContext;
            if (sessionId)
                c.sessionId = *sessionId;
            HistoryDataGathering* p = static_cast<HistoryDataGathering*>(hdgContext);
            return p->setValue(c, historizing, value);
        }
    }

public:
    /*!
        \brief HistoryDataGathering
        \param initialNodeIdStoreSize
    */
    HistoryDataGathering() {}

    virtual ~HistoryDataGathering() { deleteMembers(); }
    /*!
        \brief setDefault
        \param initialNodeIdStoreSize
    */
    void setDefault(size_t initialNodeIdStoreSize = 100)
    {
        _gathering = UA_HistoryDataGathering_Default(initialNodeIdStoreSize);  // map to default memory historian
    }
    /*!
        \brief initialise
        map to class methods
    */
    void initialise()
    {
        _gathering.registerNodeId        = _registerNodeId;
        _gathering.deleteMembers         = _deleteMembers;
        _gathering.getHistorizingSetting = _getHistorizingSetting;
        _gathering.setValue              = _setValue;
        _gathering.startPoll             = _startPoll;
        _gathering.stopPoll              = _stopPoll;
        _gathering.updateNodeIdSetting   = _updateNodeIdSetting;
        _gathering.context               = this;
    }
    /*!
        \brief gathering
        \return
    */
    UA_HistoryDataGathering& gathering() { return _gathering; }
    /*!
        \brief deleteMembers
    */
    virtual void deleteMembers() {}

    /*  This function registers a node for the gathering of historical data.

        server is the server the node lives in.
        hdgContext is the context of the UA_HistoryDataGathering.
        nodeId is the node id of the node to register.
        setting contains the gatering settings for the node to register. */
    virtual UA_StatusCode registerNodeId(Context& /*c*/, const UA_HistorizingNodeIdSettings /*setting*/) { return 0; }

    /*  This function stops polling a node for value changes.

        server is the server the node lives in.
        hdgContext is the context of the UA_HistoryDataGathering.
        nodeId is id of the node for which polling shall be stopped.
        setting contains the gatering settings for the node. */
    virtual UA_StatusCode stopPoll(Context& /*c*/) { return UA_STATUSCODE_GOOD; }

    /*  This function starts polling a node for value changes.

        server is the server the node lives in.
        hdgContext is the context of the UA_HistoryDataGathering.
        nodeId is the id of the node for which polling shall be started. */
    virtual UA_StatusCode startPoll(Context& /*c*/) { return UA_STATUSCODE_GOOD; }

    /*  This function modifies the gathering settings for a node.

        server is the server the node lives in.
        hdgContext is the context of the UA_HistoryDataGathering.
        nodeId is the node id of the node for which gathering shall be modified.
        setting contains the new gatering settings for the node. */
    virtual UA_Boolean updateNodeIdSetting(Context& /*c*/, const UA_HistorizingNodeIdSettings /*setting*/)
    {
        return UA_FALSE;
    }

    /*  Returns the gathering settings for a node.

        server is the server the node lives in.
        hdgContext is the context of the UA_HistoryDataGathering.
        nodeId is the node id of the node for which the gathering settings shall
              be retrieved. */
    virtual const UA_HistorizingNodeIdSettings* getHistorizingSetting(Context& /*c*/) { return nullptr; }

    /*  Sets a DataValue for a node in the historical data storage.

        server is the server the node lives in.
        hdgContext is the context of the UA_HistoryDataGathering.
        sessionId and sessionContext identify the session which wants to set this value.
        nodeId is the node id of the node for which a value shall be set.
        historizing is the historizing flag of the node identified by nodeId.
        value is the value to set in the history data storage. */
    virtual void setValue(Context& /*c*/, UA_Boolean historizing, const UA_DataValue* /*value*/) {}
};

/*!
    \brief The HistoryDatabase class
    This is the historian storage database
*/
class HistoryDataBackend
{
public:
    // Call back context common to most call backs - move common bits into one structure so we can simplify calls
    // and maybe do extra magic
    //
    struct Context {
        Server& server;
        NodeId sessionId;
        void* sessionContext;
        NodeId nodeId;
        Context(UA_Server* s, const UA_NodeId* sId, void* sContext, const UA_NodeId* nId);
    };

private:
    UA_HistoryDataBackend _database;  // the database structure
    //
    // Define the callbacks
    static void _deleteMembers(UA_HistoryDataBackend* backend)
    {
        if (backend && backend->context) {
            HistoryDataBackend* p = static_cast<HistoryDataBackend*>(backend->context);
            p->deleteMembers();  // destructor close handles etc
        }
    }

    /*!
        \brief _serverSetHistoryData
        \param server
        \param hdbContext
        \param sessionId
        \param sessionContext
        \param nodeId
        \param historizing
        \param value
        \return
    */
    static UA_StatusCode _serverSetHistoryData(UA_Server* server,
                                               void* hdbContext,
                                               const UA_NodeId* sessionId,
                                               void* sessionContext,
                                               const UA_NodeId* nodeId,
                                               UA_Boolean historizing,
                                               const UA_DataValue* value)
    {
        if (hdbContext && sessionId) {
            Context c(server, sessionId, sessionContext, nodeId);
            HistoryDataBackend* p = static_cast<HistoryDataBackend*>(hdbContext);
            return p->serverSetHistoryData(c, historizing, value);
        }

        return UA_STATUSCODE_GOOD;  // ignore
    }

    static UA_StatusCode _getHistoryData(UA_Server* server,
                                         const UA_NodeId* sessionId,
                                         void* sessionContext,
                                         const UA_HistoryDataBackend* backend,
                                         const UA_DateTime start,
                                         const UA_DateTime end,
                                         const UA_NodeId* nodeId,
                                         size_t maxSizePerResponse,
                                         UA_UInt32 numValuesPerNode,
                                         UA_Boolean returnBounds,
                                         UA_TimestampsToReturn timestampsToReturn,
                                         UA_NumericRange range,
                                         UA_Boolean releaseContinuationPoints,
                                         const UA_ByteString* continuationPoint,
                                         UA_ByteString* outContinuationPoint,
                                         UA_HistoryData* result)
    {
        if (backend && backend->context) {
            Context c(server, sessionId, sessionContext, nodeId);
            HistoryDataBackend* p = static_cast<HistoryDataBackend*>(backend->context);
            std::string in        = fromByteString(*(const_cast<UA_ByteString*>(continuationPoint)));
            std::string out;

            UA_StatusCode ret     = p->getHistoryData(c,
                                                  start,
                                                  end,
                                                  maxSizePerResponse,
                                                  numValuesPerNode,
                                                  returnBounds,
                                                  timestampsToReturn,
                                                  range,
                                                  releaseContinuationPoints,
                                                  in,
                                                  out,
                                                  result);
            *outContinuationPoint = UA_BYTESTRING(const_cast<char*>(out.c_str()));
            return ret;
        }
        return UA_STATUSCODE_GOOD;  // ignore
    }
    //
    /*!
        \brief _getDateTimeMatch
        \param server
        \param hdbContext
        \param sessionId
        \param sessionContext
        \param nodeId
        \param timestamp
        \param strategy
        \return
    */
    static size_t _getDateTimeMatch(UA_Server* server,
                                    void* hdbContext,
                                    const UA_NodeId* sessionId,
                                    void* sessionContext,
                                    const UA_NodeId* nodeId,
                                    const UA_DateTime timestamp,
                                    const MatchStrategy strategy)
    {
        if (hdbContext && sessionId) {
            Context c(server, sessionId, sessionContext, nodeId);
            HistoryDataBackend* p = static_cast<HistoryDataBackend*>(hdbContext);
            return p->getDateTimeMatch(c, timestamp, strategy);
        }
        return 0;
    }

    /*!
        \brief _getEnd
        \param server
        \param hdbContext
        \param sessionId
        \param sessionContext
        \param nodeId
        \return
    */
    static size_t _getEnd(UA_Server* server,
                          void* hdbContext,
                          const UA_NodeId* sessionId,
                          void* sessionContext,
                          const UA_NodeId* nodeId)
    {
        if (hdbContext && sessionId) {
            Context c(server, sessionId, sessionContext, nodeId);
            HistoryDataBackend* p = static_cast<HistoryDataBackend*>(hdbContext);
            return p->getEnd(c);
        }
        return 0;
    }
    /*!
        \brief _lastIndex
        \param server
        \param hdbContext
        \param sessionId
        \param sessionContext
        \param nodeId
        \return
    */
    static size_t _lastIndex(UA_Server* server,
                             void* hdbContext,
                             const UA_NodeId* sessionId,
                             void* sessionContext,
                             const UA_NodeId* nodeId)
    {
        if (hdbContext && sessionId) {

            Context c(server, sessionId, sessionContext, nodeId);
            HistoryDataBackend* p = static_cast<HistoryDataBackend*>(hdbContext);
            return p->lastIndex(c);
        }
        return 0;
    }

    /*!
        \brief _firstIndex
        \param server
        \param hdbContext
        \param sessionId
        \param sessionContext
        \param nodeId
        \return
    */
    static size_t _firstIndex(UA_Server* server,
                              void* hdbContext,
                              const UA_NodeId* sessionId,
                              void* sessionContext,
                              const UA_NodeId* nodeId)
    {
        if (hdbContext && sessionId) {
            Context c(server, sessionId, sessionContext, nodeId);
            HistoryDataBackend* p = static_cast<HistoryDataBackend*>(hdbContext);
            return p->firstIndex(c);
        }
        return 0;
    }

    /*!
        \brief _resultSize
        \param server
        \param hdbContext
        \param sessionId
        \param sessionContext
        \param nodeId
        \param startIndex
        \param endIndex
        \return
    */
    static size_t _resultSize(UA_Server* server,
                              void* hdbContext,
                              const UA_NodeId* sessionId,
                              void* sessionContext,
                              const UA_NodeId* nodeId,
                              size_t startIndex,
                              size_t endIndex)
    {
        if (hdbContext && sessionId) {
            Context c(server, sessionId, sessionContext, nodeId);
            HistoryDataBackend* p = static_cast<HistoryDataBackend*>(hdbContext);
            return p->resultSize(c, startIndex, endIndex);
        }
        return 0;
    }

    /*!
        \brief _copyDataValues
        \param server
        \param hdbContext
        \param sessionId
        \param sessionContext
        \param nodeId
        \param startIndex
        \param endIndex
        \param reverse
        \param valueSize
        \param range
        \param releaseContinuationPoints
        \param continuationPoint
        \param outContinuationPoint
        \param providedValues
        \param values
        \return
    */
    static UA_StatusCode _copyDataValues(UA_Server* server,
                                         void* hdbContext,
                                         const UA_NodeId* sessionId,
                                         void* sessionContext,
                                         const UA_NodeId* nodeId,
                                         size_t startIndex,
                                         size_t endIndex,
                                         UA_Boolean reverse,
                                         size_t valueSize,
                                         UA_NumericRange range,
                                         UA_Boolean releaseContinuationPoints,
                                         const UA_ByteString* continuationPoint,
                                         UA_ByteString* outContinuationPoint,
                                         size_t* providedValues,
                                         UA_DataValue* values)
    {
        if (hdbContext && sessionId) {
            Context c(server, sessionId, sessionContext, nodeId);
            HistoryDataBackend* p = static_cast<HistoryDataBackend*>(hdbContext);
            std::string in        = fromByteString(*(const_cast<UA_ByteString*>(continuationPoint)));
            std::string out;
            UA_StatusCode ret     = p->copyDataValues(c,
                                                  startIndex,
                                                  endIndex,
                                                  reverse,
                                                  valueSize,
                                                  range,
                                                  releaseContinuationPoints,
                                                  in,
                                                  out,
                                                  providedValues,
                                                  values);
            *outContinuationPoint = UA_BYTESTRING(const_cast<char*>(out.c_str()));
            return ret;
        }
        return 0;
    }

    /*!
        \brief _getDataValue
        \param server
        \param hdbContext
        \param sessionId
        \param sessionContext
        \param nodeId
        \param index
        \return
    */
    static const UA_DataValue* _getDataValue(UA_Server* server,
                                             void* hdbContext,
                                             const UA_NodeId* sessionId,
                                             void* sessionContext,
                                             const UA_NodeId* nodeId,
                                             size_t index)
    {
        if (hdbContext && sessionId) {
            Context c(server, sessionId, sessionContext, nodeId);
            HistoryDataBackend* p = static_cast<HistoryDataBackend*>(hdbContext);
            return p->getDataValue(c, index);
        }
        return nullptr;
    }

    static UA_Boolean _boundSupported(UA_Server* server,
                                      void* hdbContext,
                                      const UA_NodeId* sessionId,
                                      void* sessionContext,
                                      const UA_NodeId* nodeId)
    {
        if (hdbContext && sessionId) {
            Context c(server, sessionId, sessionContext, nodeId);
            HistoryDataBackend* p = static_cast<HistoryDataBackend*>(hdbContext);
            return p->boundSupported(c);
        }
        return UA_FALSE;
    }

    static UA_Boolean _timestampsToReturnSupported(UA_Server* server,
                                                   void* hdbContext,
                                                   const UA_NodeId* sessionId,
                                                   void* sessionContext,
                                                   const UA_NodeId* nodeId,
                                                   const UA_TimestampsToReturn timestampsToReturn)
    {
        if (hdbContext && sessionId) {
            Context c(server, sessionId, sessionContext, nodeId);
            HistoryDataBackend* p = static_cast<HistoryDataBackend*>(hdbContext);
            return p->timestampsToReturnSupported(c, timestampsToReturn);
        }
        return UA_FALSE;
    }

    static UA_StatusCode _insertDataValue(UA_Server* server,
                                          void* hdbContext,
                                          const UA_NodeId* sessionId,
                                          void* sessionContext,
                                          const UA_NodeId* nodeId,
                                          const UA_DataValue* value)
    {
        if (hdbContext && sessionId) {
            Context c(server, sessionId, sessionContext, nodeId);
            HistoryDataBackend* p = static_cast<HistoryDataBackend*>(hdbContext);
            return p->insertDataValue(c, value);
        }
        return 0;
    }
    static UA_StatusCode _replaceDataValue(UA_Server* server,
                                           void* hdbContext,
                                           const UA_NodeId* sessionId,
                                           void* sessionContext,
                                           const UA_NodeId* nodeId,
                                           const UA_DataValue* value)
    {
        if (hdbContext && sessionId) {
            Context c(server, sessionId, sessionContext, nodeId);
            HistoryDataBackend* p = static_cast<HistoryDataBackend*>(hdbContext);
            return p->replaceDataValue(c, value);
        }
        return 0;
    }
    static UA_StatusCode _updateDataValue(UA_Server* server,
                                          void* hdbContext,
                                          const UA_NodeId* sessionId,
                                          void* sessionContext,
                                          const UA_NodeId* nodeId,
                                          const UA_DataValue* value)
    {
        if (hdbContext && sessionId) {
            Context c(server, sessionId, sessionContext, nodeId);
            HistoryDataBackend* p = static_cast<HistoryDataBackend*>(hdbContext);
            return p->updateDataValue(c, value);
        }
        return 0;
    }
    /*!
        \brief _removeDataValue
        \param server
        \param hdbContext
        \param sessionId
        \param sessionContext
        \param nodeId
        \param startTimestamp
        \param endTimestamp
        \return
    */
    static UA_StatusCode _removeDataValue(UA_Server* server,
                                          void* hdbContext,
                                          const UA_NodeId* sessionId,
                                          void* sessionContext,
                                          const UA_NodeId* nodeId,
                                          UA_DateTime startTimestamp,
                                          UA_DateTime endTimestamp)
    {
        if (hdbContext && sessionId) {
            Context c(server, sessionId, sessionContext, nodeId);
            HistoryDataBackend* p = static_cast<HistoryDataBackend*>(hdbContext);
            return p->removeDataValue(c, startTimestamp, endTimestamp);
        }
        return 0;
    }
    //
public:
    HistoryDataBackend() { memset(&_database, 0, sizeof(_database)); }

    void setMemory(size_t nodes = 100, size_t size = 1000000) { _database = UA_HistoryDataBackend_Memory(nodes, size); }

    /*!
        \brief initialise to use class methods
    */
    void initialise()
    {
        memset(&_database, 0, sizeof(_database));
        _database.context = this;
        // set up the static callback methods
        _database.boundSupported              = _boundSupported;
        _database.copyDataValues              = _copyDataValues;
        _database.deleteMembers               = _deleteMembers;
        _database.firstIndex                  = _firstIndex;
        _database.getDataValue                = _getDataValue;
        _database.getDateTimeMatch            = _getDateTimeMatch;
        _database.getEnd                      = _getEnd;
        _database.getHistoryData              = _getHistoryData;
        _database.insertDataValue             = _insertDataValue;
        _database.lastIndex                   = _lastIndex;
        _database.removeDataValue             = _removeDataValue;
        _database.replaceDataValue            = _replaceDataValue;
        _database.resultSize                  = _resultSize;
        _database.serverSetHistoryData        = _serverSetHistoryData;
        _database.timestampsToReturnSupported = _timestampsToReturnSupported;
        _database.updateDataValue             = _updateDataValue;
    }
    /*!
        \brief ~HistoryDatabase
    */

    virtual ~HistoryDataBackend()
    {
        deleteMembers();  // clean up
    }

    UA_HistoryDataBackend& database() { return _database; }

    /*!
        \brief deleteMembers
    */
    virtual void deleteMembers() {}

    /*  This function sets a DataValue for a node in the historical data storage.

        server is the server the node lives in.
        hdbContext is the context of the UA_HistoryDataBackend.
        sessionId and sessionContext identify the session that wants to read historical data.
        nodeId is the node for which the value shall be stored.
        value is the value which shall be stored.
        historizing is the historizing flag of the node identified by nodeId.
        If sessionId is NULL, the historizing flag is invalid and must not be used. */
    virtual UA_StatusCode serverSetHistoryData(Context& /*c*/, bool /*historizing*/, const UA_DataValue* /*value*/)
    {
        return UA_STATUSCODE_GOOD;
    }

    /*  This function is the high level interface for the ReadRaw operation. Set
        it to NULL if you use the low level API for your plugin. It should be
        used if the low level interface does not suite your database. It is more
        complex to implement the high level interface but it also provide more
        freedom. If you implement this, then set all low level api function
        pointer to NULL.

        server is the server the node lives in.
        hdbContext is the context of the UA_HistoryDataBackend.
        sessionId and sessionContext identify the session that wants to read historical data.
        backend is the HistoryDataBackend whose storage is to be queried.
        start is the start time of the HistoryRead request.
        end is the end time of the HistoryRead request.
        nodeId is the node id of the node for which historical data is requested.
        maxSizePerResponse is the maximum number of items per response the server can provide.
        numValuesPerNode is the maximum number of items per response the client wants to receive.
        returnBounds determines if the client wants to receive bounding values.
        timestampsToReturn contains the time stamps the client is interested in.
        range is the numeric range the client wants to read.
        releaseContinuationPoints determines if the continuation points shall be released.
        continuationPoint is the continuation point the client wants to release or start from.
        outContinuationPoint is the continuation point that gets passed to the
                            client by the HistoryRead service.
        result contains the result histoy data that gets passed to the client. */
    virtual UA_StatusCode getHistoryData(Context& /*c*/,
                                         const UA_DateTime /*start*/,
                                         const UA_DateTime /*end*/,
                                         size_t /*maxSizePerResponse*/,
                                         UA_UInt32 /*numValuesPerNode*/,
                                         UA_Boolean /*returnBounds*/,
                                         UA_TimestampsToReturn /*timestampsToReturn*/,
                                         UA_NumericRange /*range*/,
                                         UA_Boolean /*releaseContinuationPoints*/,
                                         std::string& /*continuationPoint*/,
                                         std::string& /*outContinuationPoint*/,
                                         UA_HistoryData* /*result*/)
    {
        return UA_STATUSCODE_GOOD;
    }

    /*  This function is part of the low level HistoryRead API. It returns the
        index of a value in the database which matches certain criteria.

        server is the server the node lives in.
        hdbContext is the context of the UA_HistoryDataBackend.
        sessionId and sessionContext identify the session that wants to read historical data.
        nodeId is the node id of the node for which the matching value shall be found.
        timestamp is the timestamp of the requested index.
        strategy is the matching strategy which shall be applied in finding the index. */
    virtual size_t getDateTimeMatch(Context& c, const UA_DateTime /*timestamp*/, const MatchStrategy /*strategy*/)
    {
        return 0;
    }

    /*  This function is part of the low level HistoryRead API. It returns the
        index of the element after the last valid entry in the database for a
        node.

        server is the server the node lives in.
        hdbContext is the context of the UA_HistoryDataBackend.
        sessionId and sessionContext identify the session that wants to read historical data.
        nodeId is the node id of the node for which the end of storage shall be returned. */
    virtual size_t getEnd(Context& c) { return 0; }

    /*  This function is part of the low level HistoryRead API. It returns the
        index of the last element in the database for a node.

        server is the server the node lives in.
        hdbContext is the context of the UA_HistoryDataBackend.
        sessionId and sessionContext identify the session that wants to read historical data.
        nodeId is the node id of the node for which the index of the last element
              shall be returned. */
    virtual size_t lastIndex(Context& c) { return 0; }

    /*  This function is part of the low level HistoryRead API. It returns the
        index of the first element in the database for a node.

        server is the server the node lives in.
        hdbContext is the context of the UA_HistoryDataBackend.
        sessionId and sessionContext identify the session that wants to read historical data.
        nodeId is the node id of the node for which the index of the first
              element shall be returned. */

    virtual size_t firstIndex(Context& c) { return 0; }

    /*  This function is part of the low level HistoryRead API. It returns the
        number of elements between startIndex and endIndex including both.

        server is the server the node lives in.
        hdbContext is the context of the UA_HistoryDataBackend.
        sessionId and sessionContext identify the session that wants to read historical data.
        nodeId is the node id of the node for which the number of elements shall be returned.
        startIndex is the index of the first element in the range.
        endIndex is the index of the last element in the range. */
    virtual size_t resultSize(Context& c, size_t /*startIndex*/, size_t /*endIndex*/) { return 0; }

    /*  This function is part of the low level HistoryRead API. It copies data
        values inside a certain range into a buffer.

        server is the server the node lives in.
        hdbContext is the context of the UA_HistoryDataBackend.
        sessionId and sessionContext identify the session that wants to read historical data.
        nodeId is the node id of the node for which the data values shall be copied.
        startIndex is the index of the first value in the range.
        endIndex is the index of the last value in the range.
        reverse determines if the values shall be copied in reverse order.
        valueSize is the maximal number of data values to copy.
        range is the numeric range which shall be copied for every data value.
        releaseContinuationPoints determines if the continuation points shall be released.
        continuationPoint is a continuation point the client wants to release or start from.
        outContinuationPoint is a continuation point which will be passed to the client.
        providedValues contains the number of values that were copied.
        values contains the values that have been copied from the database. */
    virtual UA_StatusCode copyDataValues(Context& c,
                                         size_t /*startIndex*/,
                                         size_t /*endIndex*/,
                                         UA_Boolean /*reverse*/,
                                         size_t /*valueSize*/,
                                         UA_NumericRange /*range*/,
                                         UA_Boolean /*releaseContinuationPoints*/,
                                         std::string& /*in*/,
                                         std::string& /*out*/,
                                         size_t* /*providedValues*/,
                                         UA_DataValue* /*values*/)
    {
        return 0;
    }

    /*  This function is part of the low level HistoryRead API. It returns the
        data value stored at a certain index in the database.

        server is the server the node lives in.
        hdbContext is the context of the UA_HistoryDataBackend.
        sessionId and sessionContext identify the session that wants to read historical data.
        nodeId is the node id of the node for which the data value shall be returned.
        index is the index in the database for which the data value is requested. */
    virtual const UA_DataValue* getDataValue(Context& c, size_t /*index*/) { return nullptr; }

    /*  This function returns UA_TRUE if the backend supports returning bounding
        values for a node. This function is mandatory.

        server is the server the node lives in.
        hdbContext is the context of the UA_HistoryDataBackend.
        sessionId and sessionContext identify the session that wants to read
                 historical data.
        nodeId is the node id of the node for which the capability to return
              bounds shall be queried. */
    virtual UA_Boolean boundSupported(Context& /*c*/) { return UA_FALSE; }

    /*  This function returns UA_TRUE if the backend supports returning the
        requested timestamps for a node. This function is mandatory.

        server is the server the node lives in.
        hdbContext is the context of the UA_HistoryDataBackend.
        sessionId and sessionContext identify the session that wants to read historical data.
        nodeId is the node id of the node for which the capability to return
              certain timestamps shall be queried. */
    virtual UA_Boolean timestampsToReturnSupported(Context& /*c*/, const UA_TimestampsToReturn /*timestampsToReturn*/)
    {
        return UA_FALSE;
    }

    /*!
        \brief insertDataValue
        \return
    */
    virtual UA_StatusCode insertDataValue(Context& /*c*/, const UA_DataValue* /*value*/) { return 0; }
    /*!
        \brief replaceDataValue
        \return
    */
    virtual UA_StatusCode replaceDataValue(Context& /*c*/, const UA_DataValue* /*value*/) { return 0; }
    /*!
        \brief updateDataValue
        \return
    */
    virtual UA_StatusCode updateDataValue(Context& /*c*/, const UA_DataValue* /*value*/) { return 0; }
    /*!
        \brief removeDataValue
        \return
    */
    virtual UA_StatusCode removeDataValue(Context& /*c*/, UA_DateTime /*startTimestamp*/, UA_DateTime /*endTimestamp*/)
    {
        return 0;
    }
};

class HistoryDatabase
{

    struct Context {
        Server& server;
        NodeId sessionId;
        void* sessionContext;
        NodeId nodeId;
        Context(UA_Server* s, const UA_NodeId* sId, void* sContext, const UA_NodeId* nId);
    };

    UA_HistoryDatabase _database;
    //
    static void _deleteMembers(UA_HistoryDatabase* hdb)
    {
        if (hdb && hdb->context) {
            HistoryDatabase* p = static_cast<HistoryDatabase*>(hdb->context);
            p->deleteMembers();
        }
    }

    /*  This function will be called when a nodes value is set.
        Use this to insert data into your database(s) if polling is not suitable
        and you need to get all data changes.
        Set it to NULL if you do not need it.

        server is the server this node lives in.
        hdbContext is the context of the UA_HistoryDatabase.
        sessionId and sessionContext identify the session which set this value.
        nodeId is the node id for which data was set.
        historizing is the nodes boolean flag for historizing
        value is the new value. */
    static void _setValue(UA_Server* server,
                          void* hdbContext,
                          const UA_NodeId* sessionId,
                          void* sessionContext,
                          const UA_NodeId* nodeId,
                          UA_Boolean historizing,
                          const UA_DataValue* value)
    {
        if (hdbContext) {
            Context c(server, sessionId, sessionContext, nodeId);
            HistoryDatabase* p = static_cast<HistoryDatabase*>(hdbContext);
            p->setValue(c, historizing, value);
        }
    }

    /*  This function is called if a history read is requested with
        isRawReadModified set to false. Setting it to NULL will result in a
        response with statuscode UA_STATUSCODE_BADHISTORYOPERATIONUNSUPPORTED.

        server is the server this node lives in.
        hdbContext is the context of the UA_HistoryDatabase.
        sessionId and sessionContext identify the session which set this value.
        requestHeader, historyReadDetails, timestampsToReturn, releaseContinuationPoints
        nodesToReadSize and nodesToRead is the requested data from the client. It
                       is from the request object.
        response the response to fill for the client. If the request is ok, there
                is no need to use it. Use this to set status codes other than
                "Good" or other data. You find an already allocated
                UA_HistoryReadResult array with an UA_HistoryData object in the
                extension object in the size of nodesToReadSize. If you are not
                willing to return data, you have to delete the results array,
                set it to NULL and set the resultsSize to 0. Do not access
                historyData after that.
        historyData is a proper typed pointer array pointing in the
                   UA_HistoryReadResult extension object. use this to provide
                   result data to the client. Index in the array is the same as
                   in nodesToRead and the UA_HistoryReadResult array. */
    static void _readRaw(UA_Server* server,
                         void* hdbContext,
                         const UA_NodeId* sessionId,
                         void* sessionContext,
                         const UA_RequestHeader* requestHeader,
                         const UA_ReadRawModifiedDetails* historyReadDetails,
                         UA_TimestampsToReturn timestampsToReturn,
                         UA_Boolean releaseContinuationPoints,
                         size_t nodesToReadSize,
                         const UA_HistoryReadValueId* nodesToRead,
                         UA_HistoryReadResponse* response,
                         UA_HistoryData* const* const historyData)
    {
        if (hdbContext) {
            Context c(server, sessionId, sessionContext, sessionId);
            HistoryDatabase* p = static_cast<HistoryDatabase*>(hdbContext);
            p->readRaw(c,
                       requestHeader,
                       historyReadDetails,
                       timestampsToReturn,
                       releaseContinuationPoints,
                       nodesToReadSize,
                       nodesToRead,
                       response,
                       historyData);
        }
    }

    static void _updateData(UA_Server* server,
                            void* hdbContext,
                            const UA_NodeId* sessionId,
                            void* sessionContext,
                            const UA_RequestHeader* requestHeader,
                            const UA_UpdateDataDetails* details,
                            UA_HistoryUpdateResult* result)
    {
        if (hdbContext) {
            Context c(server, sessionId, sessionContext, sessionId);
            HistoryDatabase* p = static_cast<HistoryDatabase*>(hdbContext);
            p->updateData(c, requestHeader, details, result);
        }
    }

    static void _deleteRawModified(UA_Server* server,
                                   void* hdbContext,
                                   const UA_NodeId* sessionId,
                                   void* sessionContext,
                                   const UA_RequestHeader* requestHeader,
                                   const UA_DeleteRawModifiedDetails* details,
                                   UA_HistoryUpdateResult* result)
    {
        if (hdbContext) {
            Context c(server, sessionId, sessionContext, sessionId);
            HistoryDatabase* p = static_cast<HistoryDatabase*>(hdbContext);
            p->deleteRawModified(c, requestHeader, details, result);
        }
    }

public:
    HistoryDatabase() {}

    virtual ~HistoryDatabase() {}

    UA_HistoryDatabase& database() { return _database; }

    virtual void deleteMembers() {}

    /*  This function will be called when a nodes value is set.
        Use this to insert data into your database(s) if polling is not suitable
        and you need to get all data changes.
        Set it to NULL if you do not need it.

        server is the server this node lives in.
        hdbContext is the context of the UA_HistoryDatabase.
        sessionId and sessionContext identify the session which set this value.
        nodeId is the node id for which data was set.
        historizing is the nodes boolean flag for historizing
        value is the new value. */
    virtual void setValue(Context& /*c*/, UA_Boolean /*historizing*/, const UA_DataValue* /*value*/) {}

    /*  This function is called if a history read is requested with
        isRawReadModified set to false. Setting it to NULL will result in a
        response with statuscode UA_STATUSCODE_BADHISTORYOPERATIONUNSUPPORTED.

        server is the server this node lives in.
        hdbContext is the context of the UA_HistoryDatabase.
        sessionId and sessionContext identify the session which set this value.
        requestHeader, historyReadDetails, timestampsToReturn, releaseContinuationPoints
        nodesToReadSize and nodesToRead is the requested data from the client. It
                       is from the request object.
        response the response to fill for the client. If the request is ok, there
                is no need to use it. Use this to set status codes other than
                "Good" or other data. You find an already allocated
                UA_HistoryReadResult array with an UA_HistoryData object in the
                extension object in the size of nodesToReadSize. If you are not
                willing to return data, you have to delete the results array,
                set it to NULL and set the resultsSize to 0. Do not access
                historyData after that.
        historyData is a proper typed pointer array pointing in the
                   UA_HistoryReadResult extension object. use this to provide
                   result data to the client. Index in the array is the same as
                   in nodesToRead and the UA_HistoryReadResult array. */
    virtual void readRaw(Context& /*c*/,
                         const UA_RequestHeader* /*requestHeader*/,
                         const UA_ReadRawModifiedDetails* /*historyReadDetails*/,
                         UA_TimestampsToReturn /*timestampsToReturn*/,
                         UA_Boolean /*releaseContinuationPoints*/,
                         size_t /*nodesToReadSize*/,
                         const UA_HistoryReadValueId* /*nodesToRead*/,
                         UA_HistoryReadResponse* /*response*/,
                         UA_HistoryData* const* const /*historyData*/)
    {
    }

    virtual void updateData(Context& /*c*/,
                            const UA_RequestHeader* /*requestHeader*/,
                            const UA_UpdateDataDetails* /*details*/,
                            UA_HistoryUpdateResult* /*result*/)
    {
    }

    virtual void deleteRawModified(Context& /*c*/,
                                   const UA_RequestHeader* /*requestHeader*/,
                                   const UA_DeleteRawModifiedDetails* /*details*/,
                                   UA_HistoryUpdateResult* /*result*/)
    {
    }

    /*  Add more function pointer here.
        For example for read_event, read_modified, read_processed, read_at_time */
};

/*!
    \brief The Historian class
    Base class - the C++ abstractions shallow copy the database, backend and gathering structs
    The C++ abstractions need to have a life time loger than the server
    This agregation is used to set the historian on nodes
*/

class Historian
{
protected:
    // the parts
    UA_HistoryDatabase _database;
    UA_HistoryDataBackend _backend;
    UA_HistoryDataGathering _gathering;

public:
    Historian()
    {
        memset(&_database, 0, sizeof(_database));
        memset(&_backend, 0, sizeof(_backend));
        memset(&_gathering, 0, sizeof(_gathering));
    }
    virtual ~Historian()
    {
        if (_backend.context)
            UA_HistoryDataBackend_Memory_clear(&_backend);
    }

    // accessors
    UA_HistoryDatabase& database() { return _database; }
    UA_HistoryDataGathering& gathering() { return _gathering; }
    UA_HistoryDataBackend& backend() { return _backend; }

    bool setUpdateNode(NodeId& nodeId,
                       Server& server,
                       size_t responseSize = 100,
                       size_t pollInterval = 1000,
                       void* context       = nullptr);
    bool setPollNode(NodeId& nodeId,
                     Server& server,
                     size_t responseSize = 100,
                     size_t pollInterval = 1000,
                     void* context       = nullptr);
    bool setUserNode(NodeId& nodeId,
                     Server& server,
                     size_t responseSize = 100,
                     size_t pollInterval = 1000,
                     void* context       = nullptr);
};

/*!
    \brief The MemoryHistorian class
    This is the provided in memory historian
*/
class MemoryHistorian : public Historian
{
public:
    MemoryHistorian(size_t numberNodes = 100, size_t maxValuesPerNode = 100)
    {
        gathering() = UA_HistoryDataGathering_Default(numberNodes);
        database()  = UA_HistoryDatabase_default(gathering());
        backend()   = UA_HistoryDataBackend_Memory(numberNodes, maxValuesPerNode);
    }
    ~MemoryHistorian() {}
};
}  // namespace Open62541

#endif  // HISTORYDATABASE_H
