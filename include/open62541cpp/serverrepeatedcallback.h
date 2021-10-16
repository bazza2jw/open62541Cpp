/*
 * Copyright (C) 2017 -  B. J. Hill
 *
 * This file is part of open62541 C++ classes. open62541 C++ classes are free software: you can
 * redistribute it and/or modify it under the terms of the Mozilla Public
 * License v2.0 as stated in the LICENSE file provided with open62541.
 *
 * open62541 C++ classes are distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.
 */
#ifndef SERVERREPEATEDCALLBACK_H
#define SERVERREPEATEDCALLBACK_H
#include <open62541cpp/open62541objects.h>
namespace Open62541 {

class ServerRepeatedCallback;
typedef std::function<void(ServerRepeatedCallback&)> ServerRepeatedCallbackFunc;

/*!
    \brief The ServerRepeatedCallback class
*/
class UA_EXPORT ServerRepeatedCallback
{
    Server& _server;  // parent server
    UA_UInt32 _interval = 1000;
    UA_UInt64 _id       = 0;

    ServerRepeatedCallbackFunc _func;  // functior to handle event

protected:
    UA_StatusCode _lastError = 0;

public:
    /*!
        \brief callbackFunction
        \param server
        \param data
    */
    static void callbackFunction(UA_Server* server, void* data);
    /*!
        \brief ServerRepeatedCallback
        \param s
        \param interval
    */
    ServerRepeatedCallback(Server& s, UA_UInt32 interval);
    ServerRepeatedCallback(Server& s, UA_UInt32 interval, ServerRepeatedCallbackFunc func);
    //
    //
    /*!
        \brief ~ServerRepeatedCallback
    */
    virtual ~ServerRepeatedCallback();

    /*!
        \brief start
        \return
    */
    bool start();

    /*!
        \brief changeInterval
        \param i
        \return
    */
    bool changeInterval(unsigned i);
    /*!
        \brief stop
        \return
    */
    bool stop();

    /*!
        \brief lastError
        \return
    */
    UA_StatusCode lastError() const { return _lastError; }
    /*!
        \brief server
        \return
    */
    Server& server() { return _server; }
    /*!
        \brief id
        \return
    */
    UA_UInt64 id() const { return _id; }
    /*!
        \brief callback
    */
    virtual void callback()
    {
        // if the functor is valid call it - no need to derive a handler class, unless you want to
        if (_func)
            _func(*this);
    }  // The callback
    /*!
        \brief lastOK
        \return
    */
    bool lastOK() const { return _lastError == UA_STATUSCODE_GOOD; }
};
/*!
    \brief ServerRepeatedCallbackRef
*/
typedef std::shared_ptr<ServerRepeatedCallback> ServerRepeatedCallbackRef;
}  // namespace Open62541

#endif  // SERVERREPEATEDCALLBACK_H
