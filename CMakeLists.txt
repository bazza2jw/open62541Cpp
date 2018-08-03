cmake_minimum_required(VERSION 3.5)
project(Open62541Cpp)
include (Common.cmake)
# where to install
set(CMAKE_INSTALL_PREFIX ${PIROOT}/usr/local/MRL5)
add_definitions( -DUA_LOGLEVEL=100 )
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
        src/discoveryserver.cpp
        src/monitoreditem.cpp
        src/clientsubscription.cpp
	src/discoveryserver.cpp 
	src/servernodetree.cpp
        )

include_directories(
        ./include
)

add_library(Open62541Cpp SHARED ${LIB_SOURCES})
install(TARGETS Open62541Cpp DESTINATION lib)
# add boost to target
if(Boost_FOUND)
    target_link_libraries(Open62541Cpp ${Boost_LIBRARIES})
endif()

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
        include/trace.h
        include/discoveryserver.h
        include/monitoreditem.h
        include/clientsubscription.h
 	include/trace.h 
	include/discoveryserver.h 
        )

install(FILES ${LIB_HEADERS} DESTINATION include/Open62541Cpp)



