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
#ifndef CLIENTBROWSER_H
#define CLIENTBROWSER_H
#include <open62541cpp/open62541client.h>
namespace Open62541 {

/*!
      \brief The ClientBrowser class
      Browse nodes helper.
*/

class ClientBrowser : public Browser<Client>
{
    //
public:
    /*!
        \brief ClientBrowser
        \param c client connection
    */
    ClientBrowser(Client& c)
        : Browser(c)
    {
    }
    /*!
        \brief browse
        \param start node ID
    */
    void browse(const NodeId &start)
    {
        list().clear();
        if (obj().client())
            UA_Client_forEachChildNodeCall(obj().client(), start, browseIter, (void*)this);
    }
};

}  // namespace Open62541
#endif  // CLIENTBROWSER_H
