cmake_minimum_required(VERSION 3.5)

## project name
project(open62541cpp)

## library name
set(OPEN62541_CPP Open62541Cpp)

## build settings
set(VERSION_MAJOR 0)
set(VERSION_MINOR 3)
set(VERSION_PATCH 0)

set(CLIENT_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

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

add_subdirectory(src)
add_subdirectory(include)

if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

