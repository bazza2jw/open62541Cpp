#ifndef SERVERTIMEDCALLBACK_H
#define SERVERTIMEDCALLBACK_H
#include <open62541cpp/open62541objects.h>
namespace Open62541 {

class ServerTimedCallback;
typedef std::function<void(ServerTimedCallback&)> ServerTimedCallbackFunc;
/*!
    \brief The SeverRepeatedCallback class
*/
class UA_EXPORT ServerTimedCallback
{
    Server& _server;            // parent server
    UA_DateTime _interval = 0;  // must use times based on monotonic for this to work
    UA_UInt64 _id         = 0;
    ServerTimedCallbackFunc _func;               // functor to handle event
    static std::set<ServerTimedCallback*> _map;  // set of active timers

protected:
    UA_StatusCode _lastError = 0;

public:
    /*!
        \brief callbackFunction
        \param server
        \param data
    */
    static void callbackFunction(UA_Server* server, void* data);
    /*!
        \brief SeverRepeatedCallback
        \param s
        \param interval
    */

    ServerTimedCallback(Server& s, unsigned delay = 0);
    ServerTimedCallback(Server& s, ServerTimedCallbackFunc func, unsigned delay = 0);
    //
    static std::set<ServerTimedCallback*>& map() { return _map; }  // timer map
    //
    /*!
        \brief ~SeverRepeatedCallback
    */
    virtual ~ServerTimedCallback();

    /*!
        \brief start
        \return
    */
    bool start();

    /*!
     * \brief stop
     * \return
     */
    bool stop();

    /*!
        \brief lastError
        \return
    */
    UA_StatusCode lastError() const { return _lastError; }
    /*!
        \brief server
        \return
    */
    Server& server() { return _server; }
    /*!
        \brief id
        \return
    */
    UA_UInt64 id() const { return _id; }
    /*!
        \brief callback
    */
    virtual void callback()
    {
        // if the functor is valid call it - no need to derive a handler class, unless you want to
        if (_func)
            _func(*this);
    }  // The callback
    /*!
        \brief lastOK
        \return
    */
    bool lastOK() const { return _lastError == UA_STATUSCODE_GOOD; }

    // Accessors
    /*!
     * \brief interval
     * \return
     */
    UA_DateTime interval() const { return _interval; }
    /*!
     * \brief setInterval
     * \param i
     */
    void setInterval(UA_DateTime i) { _interval = i; }
    /*!
     * \brief addSeconds
     * \param i
     */
    void addSeconds(unsigned i) { _interval += (i * UA_DATETIME_SEC); }
    /*!
     * \brief addMilliSeconds
     * \param i
     */
    void addMilliSeconds(unsigned i) { _interval += (i * UA_DATETIME_MSEC); }
};
/*!
    \brief ServerTimedCallbackRef
*/
typedef std::shared_ptr<ServerTimedCallback> ServerTimedCallbackRef;
}  // namespace Open62541
#endif  // SERVERTIMEDCALLBACK_H
