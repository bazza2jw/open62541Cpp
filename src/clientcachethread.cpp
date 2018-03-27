#include "clientcachethread.h"

/*!
    \brief Open62541::ClientCacheThread::start
    \return
*/
bool Open62541::ClientCacheThread::start() {

    try
    {
    _thread = std::thread(
    [this] {
        _running = true;
        while (_running) {
            _cache.process();
        }
    }
              );
    }
    catch(...)
    {
        return false;
    }
    return true;
}

/*!
    \brief Open62541::ClientCacheThread::stop
    \return
*/
bool Open62541::ClientCacheThread::stop() {
    _running = false;
    _thread.join();
    return true;
}
