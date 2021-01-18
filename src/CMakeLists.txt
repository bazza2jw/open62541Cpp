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
    historydatabase.cpp
    )

set(LIB_HEADERS
    ../include/open62541.h
    ../include/open62541objects.h
    ../include/open62541client.h
    ../include/open62541server.h
    ../include/propertytree.h
    ../include/clientcache.h
    ../include/clientcachethread.h
    ../include/nodecontext.h
    ../include/servermethod.h
    ../include/serverrepeatedcallback.h
    ../include/serverobjecttype.h
    ../include/serverbrowser.h
    ../include/servernodetree.h
    ../include/clientnodetree.h
    ../include/clientbrowser.h
    ../include/monitoreditem.h
    ../include/clientsubscription.h
    ../include/open62541cpp_trace.h
    ../include/discoveryserver.h
    ../include/historydatabase.h
    )

## library name
set(OPEN62541_CPP open62541cpp)

# Building shared library
add_library(               open62541cpp SHARED ${LIB_SOURCES} ${LIB_HEADERS})
target_compile_definitions(open62541cpp PRIVATE -DUA_DYNAMIC_LINKING_EXPORT)
target_include_directories(open62541cpp PUBLIC ../include)
set_target_properties(     open62541cpp PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
# add boost to target
include(../Common.cmake)
add_boost_lib(${OPEN62541_CPP})
set_build_system_option(${OPEN62541_CPP})

## set the shared library soname
set_target_properties(${OPEN62541_CPP} PROPERTIES
        VERSION   ${PACKAGE_VERSION}
        SOVERSION ${VERSION_MAJOR})

include(CMakePackageConfigHelpers)
configure_package_config_file(open62541cppConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/open62541cppConfig.cmake
        INSTALL_DESTINATION ${LIB_INSTALL_DIR}/${OPEN62541_CPP}/cmake
        PATH_VARS INCLUDE_INSTALL_DIR LIB_INSTALL_DIR
        )

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/${OPEN62541_CPP}ConfigVersion.cmake
        VERSION ${PACKAGE_VERSION}
        COMPATIBILITY SameMajorVersion)

install(FILES ${LIB_HEADERS} DESTINATION include/open62541cpp)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${OPEN62541_CPP}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${OPEN62541_CPP}ConfigVersion.cmake
        DESTINATION ${LIB_INSTALL_DIR}/${OPEN62541_CPP}/cmake )

## install the shared library
install(TARGETS ${OPEN62541_CPP}
        ARCHIVE  DESTINATION lib
        LIBRARY  DESTINATION lib
        RUNTIME  DESTINATION bin
        INCLUDES DESTINATION include
        )
