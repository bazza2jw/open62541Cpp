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
#ifndef SERVERNODETREE_H
#define SERVERNODETREE_H
#include <open62541objects.h>
namespace Open62541
{
// browsing object
/*!
    \brief The ServerBrowser class
*/
class  UA_EXPORT  ServerBrowser {
        Server &_server;
        // browser call back
        std::vector<BrowseItem> _list;
        //
        static UA_StatusCode browseIter(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle);
        //
    public:
        /*!
            \brief ServerBrowser
            \param c
        */
        ServerBrowser(Server &c) : _server(c) {}
        /*!
            \brief ~ServerBrowser
        */
        virtual ~ServerBrowser() {}
        /*!
            \brief browse
            \param start
        */
        void browse(UA_NodeId start) {
            _list.clear();
            {
                //WriteLock ll(_server.mutex());
                UA_Server_forEachChildNodeCall(_server.server(), start, browseIter, (void *) this);
            }
        }
        /*!
            \brief process
            \param childId
            \param referenceTypeId
        */
        virtual void process(UA_NodeId childId,  UA_NodeId referenceTypeId) {
            std::string s;
            int i;
            NodeId n(childId);
            if (_server.browseName(n, s, i)) {
                _list.push_back(BrowseItem(s, i, childId, referenceTypeId));
            }

        }
        std::vector<BrowseItem> &list() {
            return _list;
        }
        /*!
            \brief print
            \param os
        */
        void print(std::ostream &os) {
            for (BrowseItem &i : _list) {
                std::string s;
                int j;
                NodeId n(i.childId);
                if (_server.browseName(n, s, j)) {
                    os << toString(i.childId) << " ns:" << i.nameSpace
                       << ": "  << i.name  << " Ref:"
                       << toString(i.referenceTypeId) << std::endl;
                }
            }

        }
        /*!
            \brief find
            \param s
            \return
        */
        int find(const std::string &s) {
            int ret = -1;
            for (int i = 0; i < int(_list.size()); i++) {
                BrowseItem &b = _list[i];
                if (b.name == s)
                    return i;
            }
            return ret;
        }
};

}
#endif // SERVERNODETREE_H
