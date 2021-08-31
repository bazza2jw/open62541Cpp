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
#ifndef CLIENTCACHETHREAD_H
#define CLIENTCACHETHREAD_H

#include <thread>
#include <open62541cpp/clientcache.h>

namespace Open62541 {

/*!
    \brief The ClientCacheThread class
*/

class ClientCacheThread
{
    ClientCache& _cache;
    std::thread _thread;
    bool _running = false;

public:
    /*!
           \brief ClientCacheThread
           \param c
    */
    ClientCacheThread(ClientCache& c)
        : _cache(c)
    {
    }
    /*!
        \brief start
        \return
    */
    bool start();
    /*!
        \brief stop
        \return
    */
    bool stop();
    /*!
        \brief cache
        \return
    */
    ClientCache& cache() { return _cache; }
};
}  // namespace Open62541

#endif  // CLIENTCACHETHREAD_H
