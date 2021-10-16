
set(LIB_SOURCES
        open62541objects.cpp
        open62541client.cpp
        open62541server.cpp
        clientcache.cpp
        clientcachethread.cpp
        nodecontext.cpp
        servermethod.cpp
        serverobjecttype.cpp
        clientbrowser.cpp
        monitoreditem.cpp
        clientsubscription.cpp
        discoveryserver.cpp
        servernodetree.cpp
        historydatabase.cpp
        condition.cpp
        )

# Building shared library
add_library(${OPEN62541_CPP} SHARED ${LIB_SOURCES})
set_property(TARGET ${OPEN62541_CPP} PROPERTY POSITION_INDEPENDENT_CODE ON)

# add boost to target
include(../Common.cmake)

find_package(open62541)

target_link_libraries(${OPEN62541_CPP} PUBLIC ${Boost_LIBRARIES} open62541::open62541)

## set the shared library soname
set_target_properties(${OPEN62541_CPP} PROPERTIES
        VERSION ${PACKAGE_VERSION}
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

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${OPEN62541_CPP}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${OPEN62541_CPP}ConfigVersion.cmake
        DESTINATION ${LIB_INSTALL_DIR}/${OPEN62541_CPP}/cmake )

## install the shared library
install(TARGETS ${OPEN62541_CPP}
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINSTION include
        )
