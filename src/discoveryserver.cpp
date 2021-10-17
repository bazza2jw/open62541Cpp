/*
    Copyright (C) 2017 -  B. J. Hill

    This file is part of open62541 C++ classes. open62541 C++ classes are free software: you can
    redistribute it and/or modify it under the terms of the Mozilla Public
    License v2.0 as stated in the LICENSE file provided with open62541.

    open62541 C++ classes are distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
    A PARTICULAR PURPOSE.
*/

#include <open62541cpp/discoveryserver.h>

/*!
    \brief Open62541::DiscoveryServer::DiscoveryServer
    \param port server port
    \param url  server description
*/
Open62541::DiscoveryServer::DiscoveryServer(int port, const std::string& url)
{
    _server = UA_Server_new();
    if (_server) {
        _config = UA_Server_getConfig(_server);
        if (_config) {
            UA_ServerConfig_setMinimal(_config, port, nullptr);
            _config->applicationDescription.applicationType = UA_APPLICATIONTYPE_DISCOVERYSERVER;
            UA_String_clear(&_config->applicationDescription.applicationUri);
            _config->applicationDescription.applicationUri = UA_String_fromChars(url.c_str());
            _config->mdnsConfig.mdnsServerName = UA_String_fromChars(url.c_str());
            _config->mdnsEnabled = true;
            // See http://www.opcfoundation.org/UA/schemas/1.03/ServerCapabilities.csv
            /*  timeout in seconds when to automatically remove a registered server from
                  the list, if it doesn't re-register within the given time frame. A value
                  of 0 disables automatic removal. Default is 60 Minutes (60*60). Must be
                  bigger than 10 seconds, because cleanup is only triggered approximately
                  ervery 10 seconds. The server will still be removed depending on the
                  state of the semaphore file. */
            // config.discoveryCleanupTimeout = 60*60;
        }
    }
}

/*!
    \brief Open62541::DiscoveryServer::~DiscoveryServer
*/
Open62541::DiscoveryServer::~DiscoveryServer()
{
    if (_server)
        UA_Server_delete(_server);
    if (_config)
        delete _config;
}

/*!
    \brief Open62541::DiscoveryServer::run
*/
bool Open62541::DiscoveryServer::run()
{
    return UA_Server_run(_server, &_running) == UA_STATUSCODE_GOOD;
}
