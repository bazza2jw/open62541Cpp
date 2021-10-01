cmake_minimum_required(VERSION 3.5)

## project name
project(open62541cpp)

## library name
set(OPEN62541_CPP open62541cpp)

## build settings - try and match the c library version
set(VERSION_MAJOR 1)
set(VERSION_MINOR 1)
set(VERSION_PATCH 3)

set(PACKAGE_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})

## Set the UA log level.
IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
    add_definitions(-DUA_LOGLEVEL=100)
ELSE()
    add_definitions(-DUA_LOGLEVEL=500)
ENDIF(CMAKE_BUILD_TYPE MATCHES DEBUG)

## --- Build options ---
set(BUILD_EXAMPLES FALSE CACHE BOOL "Build example programs")

## --- C++14 build flags ---
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

IF( DEFINED CROSSBUILD )
message ("Raspberry Pi Cross Build")
set(CMAKE_INSTALL_PREFIX ${PIROOT}/usr/local)
ELSE( DEFINED CROSSBUILD )
message("Native Build")
set(CMAKE_INSTALL_PREFIX /usr/local)
ENDIF( DEFINED CROSSBUILD )
message("Install To:${CMAKE_INSTALL_PREFIX}")
find_library(OPEN62541 open62541 ${CMAKE_INSTALL_PREFIX}/lib)

set(INCLUDE_INSTALL_DIR include)
set(LIB_INSTALL_DIR lib)

## include directories
include_directories(include)

add_subdirectory(src)
add_subdirectory(include)

if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

