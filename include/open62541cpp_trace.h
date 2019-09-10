#ifndef _OPEN62541_TRACE_H
#define _OPEN62541_TRACE_H
#include <iostream>
#ifdef OPEN62541_DEBUG
    #define OPEN62541_TRACE(s) std::cerr << __FUNCTION__ << "[" << __LINE__ << "]:" << s << std::endl;
#else
    #define OPEN62541_TRACE(s)
#endif
// trace a point
#define OPEN62541_TRC OPEN62541_TRACE("");

#endif // TRACE_H
