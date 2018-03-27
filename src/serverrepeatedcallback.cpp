#include <serverrepeatedcallback.h>
#include <open62541server.h>
/*!
 *
    \brief Open62541::SeverRepeatedCallback::callbackFunction
    \param server
    \param data
*/
void Open62541::SeverRepeatedCallback::callbackFunction(UA_Server * /*server*/, void *data) {
    Open62541::SeverRepeatedCallback *p = (Open62541::SeverRepeatedCallback *)data;
    if (p) p->callback();
}

/*!
    \brief Open62541::SeverRepeatedCallback::SeverRepeatedCallback
    \param s
    \param interval
*/
Open62541::SeverRepeatedCallback::SeverRepeatedCallback(Server &s, UA_UInt32 interval)
    : _server(s),
      _interval(interval) {

}

/*!
    \brief Open62541::SeverRepeatedCallback
    This version takes a functor
    \param s
    \param interval
    \param func
*/
Open62541::SeverRepeatedCallback::SeverRepeatedCallback(Server &s, UA_UInt32 interval, SeverRepeatedCallbackFunc func)
    : _server(s),
      _interval(interval),
      _func(func)
{


}


/*!
    \brief Open62541::SeverRepeatedCallback::start
    \return
*/
bool Open62541::SeverRepeatedCallback::start() {
    WriteLock l(_server.mutex());
    _lastError = UA_Server_addRepeatedCallback(_server.server(), callbackFunction, this, _interval, &_id);
    return lastOK();
}


//
//
/*!
    \brief Open62541::SeverRepeatedCallback::~SeverRepeatedCallback
*/
Open62541::SeverRepeatedCallback::~SeverRepeatedCallback() {
    //UA_Server_removeRepeatedCallback(_server.server(), _id);
    WriteLock l(server().mutex());
    UA_Server_removeRepeatedCallback(_server.server(), _id);
}


