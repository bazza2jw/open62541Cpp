
set(LIB_SOURCES
        open62541objects.cpp
        open62541client.cpp
        open62541server.cpp
        clientcache.cpp
        clientcachethread.cpp
        nodecontext.cpp
        serverrepeatedcallback.cpp
        servertimedcallback.cpp
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


add_library(OBJS OBJECT ${LIB_SOURCES})
# Building shared library

set_property(TARGET OBJS PROPERTY POSITION_INDEPENDENT_CODE ON)
add_library(${OPEN62541_CPP} SHARED $<TARGET_OBJECTS:OBJS>)

# add boost to target
include(../Common.cmake)

find_library(OPEN62541 open62541 ${CMAKE_INSTALL_PREFIX}/lib)


if (Boost_FOUND)
    target_link_libraries(${OPEN62541_CPP} ${Boost_LIBRARIES} ${OPEN62541})
endif ()

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
