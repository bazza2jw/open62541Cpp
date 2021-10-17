# Open62541 C++ Library for Open62541 version 1.1.3

This is a set of wrapper classes for the Open62541 C OPC UA library version 1.2. The objective is to reduce the code 
required, by a considerable amount, and allow object orientated coding.

Do not assume any OPC UA feature is implemented or complete or optimally done. Support will be added as and when 
it is needed. An effort is made to include features added to the C library, eventually.

Feel free to constructively comment or contribute.

# Class Documentation

The C++ wrappers map on to the open62541 C library, keeping the same function names. Use the C documentation for explanation of what a function does. Other functions have doxygen comments but should be easy enough to follow.


# General Principles

Most UA_* types are wrapped such that for each UA_* structure there is a correspond C++ class to manage it. Hence,
UA_NodeId is wrapped with Open62541::NodeId

Assignments or copy construction always use deep copies. It is possible to shallow copy UA_* items to the 
corresponding C++ managed objects. Do not do it, unless you really, really want to.

Context pointers attached to nodes on creation are assumed to be objects derived from Open62541::NodeContext. The 
NodeContext class includes generalised handing of DataValue, node constructor/destructor, and value callbacks (that can be lambdas).

Most callbacks are wrapped as virtual functions in the associated class. When registering a callback the pointer to the associated class instance is used as the context pointer.

There is C++ style class support of Object Types.

# Node Ids

The NodeId class wraps the NodeId item. Many functions accept NULL NodeId reference as defaults and non-null references to receive NodeId values back. If you want to receive a NodeId value back the Open62541::NodeId object must be set as not being null with the notNull() member function. A NULL NodeId is wrapped in the Open62541::NodeId::Null static object.

# Building

The library is built using cmake. The examples show how to use the classes and should correspond to many of the C 
library examples. These examples can be build with `-Dwith_examples=ON`.

# Examples

The examples demonstrate how to use the library.  Some are analogs of the C library examples

## Requirements

1.  [Open62541](https://github.com/open62541/open62541). Build this library as shared and thread safe
2.  Boost

This is the configuration used for the open62541 C library:

BUILD_SHARED_LIBS                ON
 CLANG_FORMAT_EXE                 CLANG_FORMAT_EXE-NOTFOUND
 CMAKE_BUILD_TYPE                 Debug
 CMAKE_INSTALL_PREFIX             /usr/local
 MDNSD_LOGLEVEL                   300
 UA_ARCHITECTURE                  posix
 UA_BUILD_EXAMPLES                ON
 UA_BUILD_TOOLS                   ON
 UA_BUILD_UNIT_TESTS              OFF
 UA_ENABLE_AMALGAMATION           OFF
 UA_ENABLE_DA                     ON
 UA_ENABLE_DISCOVERY              ON
 UA_ENABLE_DISCOVERY_MULTICAST    ON
 UA_ENABLE_ENCRYPTION             ON
 UA_ENABLE_ENCRYPTION_MBEDTLS     OFF
 UA_ENABLE_ENCRYPTION_OPENSSL     ON
 UA_ENABLE_HISTORIZING            ON
 UA_ENABLE_METHODCALLS            ON
 UA_ENABLE_MICRO_EMB_DEV_PROFIL   OFF
 UA_ENABLE_NODEMANAGEMENT         ON
 UA_ENABLE_PARSING                ON
 UA_ENABLE_SUBSCRIPTIONS          ON
 UA_ENABLE_SUBSCRIPTIONS_ALARMS   ON
 UA_ENABLE_SUBSCRIPTIONS_EVENTS   ON
 UA_ENABLE_WEBSOCKET_SERVER       OFF
 UA_LOGLEVEL                      300
 UA_MULTITHREADING                110
 UA_NAMESPACE_ZERO                FULL
