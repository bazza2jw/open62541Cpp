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

#include "../include/discoveryserver.h"

/*!
 * \brief Open62541::DiscoveryServer::DiscoveryServer
 * \param port server port
 * \param url  server description
 */
Open62541::DiscoveryServer::DiscoveryServer(int port, const std::string &url)
{
      _config = UA_ServerConfig_new_minimal(port,nullptr);
      _config->applicationDescription.applicationType = UA_APPLICATIONTYPE_DISCOVERYSERVER;
      UA_String_deleteMembers(&_config->applicationDescription.applicationUri);
      _config->applicationDescription.applicationUri = UA_String_fromChars(url.c_str());
      _config->mdnsServerName = UA_String_fromChars("LDS");
      // See http://www.opcfoundation.org/UA/schemas/1.03/ServerCapabilities.csv
      _config->serverCapabilitiesSize = 1;
      UA_String *caps = UA_String_new();
      *caps = UA_String_fromChars("LDS");
      _config->serverCapabilities = caps;
      /* timeout in seconds when to automatically remove a registered server from
          * the list, if it doesn't re-register within the given time frame. A value
          * of 0 disables automatic removal. Default is 60 Minutes (60*60). Must be
          * bigger than 10 seconds, because cleanup is only triggered approximately
          * ervery 10 seconds. The server will still be removed depending on the
          * state of the semaphore file. */
         // config.discoveryCleanupTimeout = 60*60;
         _server = UA_Server_new(_config);
}

/*!
 * \brief Open62541::DiscoveryServer::~DiscoveryServer
 */
Open62541::DiscoveryServer::~DiscoveryServer()
{
    UA_Server_delete(_server);
    UA_ServerConfig_delete(_config);
}

/*!
 * \brief Open62541::DiscoveryServer::run
 */
bool Open62541::DiscoveryServer::run()
{
   return UA_Server_run(_server, &_running) == UA_STATUSCODE_GOOD;
}

