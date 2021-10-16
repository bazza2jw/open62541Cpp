#ifndef TRACE_HPP
#define TRACE_HPP
//
#include <iostream>
//
// Function/file/line
//
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
//
// use wxLogDebug - change to suit GUI and app framework
#ifdef TRACE_ON
#define TRC(s)                                                                                    \
    {                                                                                             \
        std::cerr << __FILE__ << ":" << __FUNCTION__ << "," << __LINE__ << "," << s << std::endl; \
    }
#define TRACE_ENTRY TRC("Entry");
#define TRACE_EXIT TRC("Exit");
#else
#define TRC(s)
#define TRACE_ENTRY
#define TRACE_EXIT
#endif

#define EXCEPT_TRC std::cerr << __FILE__ << " " << __FUNCTION__ << ":Exception Caught:" << e.what() << std::endl;
#define EXCEPT_DEF std::cerr << __FILE__ << " " << __FUNCTION__ << ":Exception Caught:" << std::endl;

//
#endif  // TRACE_HPP
