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
#ifndef SERVERBROWSER_H
#define SERVERBROWSER_H
#include <open62541cpp/open62541objects.h>
namespace Open62541 {
// browsing object
/*!
    \brief The ServerBrowser class
    Browse a server node
*/
class UA_EXPORT ServerBrowser : public Browser<Server>
{
    //
public:
    /*!
        \brief ServerBrowser
        \param c
    */
    ServerBrowser(Server& c)
        : Browser(c)
    {
    }
    /*!
        \brief browse
        \param start
    */
    void browse(const NodeId &start)
    {
        list().clear();
        {
            UA_Server_forEachChildNodeCall(obj().server(), start, browseIter, (void*)this);
        }
    }
};

}  // namespace Open62541
#endif  // SERVERBROWSER_H
