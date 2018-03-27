#ifndef SERVERREPEATEDCALLBACK_H
#define SERVERREPEATEDCALLBACK_H
#include <open62541objects.h>
namespace Open62541
{


typedef std::function<void (SeverRepeatedCallback &)> SeverRepeatedCallbackFunc;
/*!
    \brief The SeverRepeatedCallback class
*/
class  UA_EXPORT  SeverRepeatedCallback {
        Server &_server; // parent server
        UA_UInt32 _interval = 1000;
        UA_UInt64 _id = 0;
        SeverRepeatedCallbackFunc _func; // functior to handle event

    protected:
        UA_StatusCode _lastError = 0;
    public:
        /*!
            \brief callbackFunction
            \param server
            \param data
        */
        static void callbackFunction(UA_Server *server, void *data);
        /*!
            \brief SeverRepeatedCallback
            \param s
            \param interval
        */
        SeverRepeatedCallback(Server &s, UA_UInt32 interval);
        SeverRepeatedCallback(Server &s, UA_UInt32 interval, SeverRepeatedCallbackFunc func);
        //
        //
        virtual ~SeverRepeatedCallback();

        bool start();

        /*!
            \brief lastError
            \return
        */
        UA_StatusCode lastError() const {
            return _lastError;
        }
        /*!
            \brief server
            \return
        */
        Server &server() {
            return _server;
        }
        /*!
            \brief id
            \return
        */
        UA_UInt64 id() const {
            return _id;
        }
        /*!
            \brief callback
        */
        virtual void callback()
        {
            // if the functor is valid call it - no need to derive a handler class, unless you want to
            if(_func) _func(*this);
        } // The callback
        /*!
            \brief lastOK
            \return
        */
        bool lastOK() const {
            return _lastError == UA_STATUSCODE_GOOD;
        }
};
/*!
    \brief SeverRepeatedCallbackRef
*/
typedef std::shared_ptr<SeverRepeatedCallback> SeverRepeatedCallbackRef;
}


#endif // SERVERREPEATEDCALLBACK_H
