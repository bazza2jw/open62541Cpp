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
#include <open62541cpp/clientcachethread.h>

/*!
    \brief Open62541::ClientCacheThread::start
    \return true on success
*/
bool Open62541::ClientCacheThread::start()
{

    try {
        _thread = std::thread([this] {
            _running = true;
            while (_running) {
                _cache.process();
            }
        });
    }
    catch (...) {
        return false;
    }
    return true;
}

/*!
    \brief Open62541::ClientCacheThread::stop
    \return
*/
bool Open62541::ClientCacheThread::stop()
{
    _running = false;
    _thread.join();
    return true;
}
