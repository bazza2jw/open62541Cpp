cmake_minimum_required(VERSION 3.10)
project(Open62541Cpp)

set(CMAKE_CXX_STANDARD 11)
add_definitions(-D _UNIX)
set(CMAKE_INSTALL_PREFIX /usr/local/ampi)

set(LIB_SOURCES
        src/open62541client.cpp
        src/open62541.c
        src/open62541server.cpp
        src/open62541objects.cpp
        src/clientcache.cpp
        src/clientcachethread.cpp
        src/nodecontext.cpp
        src/serverrepeatedcallback.cpp
        src/servermethod.cpp
        src/serverobjecttype.cpp
        src/clientbrowser.cpp
        src/monitoreditem.cpp
        src/clientsubscription.cpp
        #src/monitoreditem.cpp
        #src/clientsubscription.cpp
        )

set(LIB_HEADERS
        include/open62541client.h
        include/open62541objects.h
        include/open62541.h
        include/open62541server.h
        include/propertytree.h
        include/clientcache.h
        include/clientcachethread.h
        include/nodecontext.h
        include/servermethod.h
        include/serverrepeatedcallback.h
        include/serverobjecttype.h
        include/serverbrowser.h
        include/servernodetree.h
        include/clientnodetree.h
        include/clientbrowser.h
        include/monitoreditem.h
        include/clientsubscription.h
        #include/monitoreditem.h
        #include/clientsubscription.h
        )

set (Boost_USE_STATIC_LIBS OFF) # enable dynamic linking
find_package(Boost COMPONENTS REQUIRED system thread)
include_directories(${Boost_INCLUDE_DIR})

include_directories(
        ./include
)

add_library(Open62541Cpp SHARED ${LIB_SOURCES})

install(TARGETS Open62541Cpp DESTINATION lib)

install(FILES ${LIB_HEADERS} DESTINATION include)
