## include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(LIB_SOURCES
        open62541.c
        open62541objects.cpp
        open62541client.cpp
        open62541server.cpp
        clientcache.cpp
        clientcachethread.cpp
        nodecontext.cpp
        serverrepeatedcallback.cpp
        servermethod.cpp
        serverobjecttype.cpp
        clientbrowser.cpp
        monitoreditem.cpp
        clientsubscription.cpp
        discoveryserver.cpp
        servernodetree.cpp
        )


add_library(OBJS OBJECT ${LIB_SOURCES})

# Building shared library

set_property(TARGET OBJS PROPERTY POSITION_INDEPENDENT_CODE ON)
add_library(${OPEN62541_CPP} SHARED $<TARGET_OBJECTS:OBJS>)

# add boost to target
if (Boost_FOUND)
    target_link_libraries(${OPEN62541_CPP} ${Boost_LIBRARIES})
endif ()

## set the shared library soname
set_target_properties(${OPEN62541_CPP} PROPERTIES
        VERSION ${CLIENT_VERSION}
        SOVERSION ${VERSION_MAJOR})

## install the shared library
install(TARGETS ${OPEN62541_CPP}
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)
