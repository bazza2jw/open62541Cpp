#include <servertimedcallback.h>
#include <open62541cpp/open62541server.h>

//
std::set<Open62541::ServerTimedCallback *> Open62541::ServerTimedCallback::_map;
//


/*!

    \brief Open62541::ServerTimedCallback::callbackFunction
    \param server
    \param data
*/
void Open62541::ServerTimedCallback::callbackFunction(UA_Server * /*server*/, void *data) {
    Open62541::ServerTimedCallback *p = (Open62541::ServerTimedCallback *)data;
        if (p) {
            p->callback();
        }
}


/*!
    \brief Open62541::ServerTimedCallback::ServerTimedCallback
    \param s
    \param interval
*/
Open62541::ServerTimedCallback::ServerTimedCallback(Server &s, unsigned delay)
    : _server(s),
      _interval(UA_DateTime_nowMonotonic() + delay) {
    _map.insert(this);

}

/*!
    \brief Open62541::ServerTimedCallback
    This version takes a functor
    \param s
    \param interval
    \param func
*/
Open62541::ServerTimedCallback::ServerTimedCallback(Server &s, ServerTimedCallbackFunc func, unsigned delay )
    : _server(s),
      _interval(UA_DateTime_nowMonotonic() + delay),
      _func(func) {
    _map.insert(this);

}


/*!
 * \brief Open62541::ServerRepeatedCallback::stop
 * \return
 */
bool Open62541::ServerTimedCallback::stop() {
    if (_id != 0) {
        if(_server.server())
        {
            WriteLock l(_server.mutex());
            UA_Server_removeRepeatedCallback(_server.server(), _id); // possible to remove
            _id = 0;
            return true;
        }
    }
    _id = 0;
    return false;
}


/*!
    \brief Open62541::ServerTimedCallback::start
    \return
*/
bool Open62541::ServerTimedCallback::start() {
    if ((_id == 0) && _server.server()) {
        WriteLock l(_server.mutex());
        _lastError = UA_Server_addTimedCallback(_server.server(), ServerTimedCallback::callbackFunction, this, _interval, &_id);
        return lastOK();
    }
    return false;
}


//
//
/*!
    \brief Open62541::ServerTimedCallback::~ServerTimedCallback
*/
Open62541::ServerTimedCallback::~ServerTimedCallback() {
    stop();
    auto i = _map.find(this);
    if(i != _map.end()) _map.erase(i);
}


