cmake_minimum_required(VERSION 3.5)

## project name
project(open62541cpp)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})

## library name
set(OPEN62541_CPP open62541cpp)

## build settings - try and match the c library version
set(VERSION_MAJOR 1)
set(VERSION_MINOR 2)
set(VERSION_PATCH 0)

set(PACKAGE_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})

## Set the UA log level.
# IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
#     add_definitions(-DUA_LOGLEVEL=100)
# ELSE()
#     add_definitions(-DUA_LOGLEVEL=500)
# ENDIF(CMAKE_BUILD_TYPE MATCHES DEBUG)

## --- Build options ---
set(BUILD_EXAMPLES FALSE CACHE BOOL "Build example programs")

## --- C++17 build flags ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(INCLUDE_INSTALL_DIR include)
set(LIB_INSTALL_DIR lib)

set(UA_CPP_SANITIZE ON)

if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND UA_CPP_SANITIZE AND
        "x${CMAKE_C_COMPILER_ID}" STREQUAL "xClang")
    # Add default sanitizer settings when using clang and Debug build.
    # This allows e.g. CLion to find memory locations for SegFaults
    message(STATUS "Sanitizer enabled")
    set(SANITIZER_FLAGS "-g -fno-omit-frame-pointer -gline-tables-only -fsanitize=address -fsanitize-address-use-after-scope -fsanitize=leak -fsanitize=undefined")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10.0)
        set(SANITIZER_FLAGS "${SANITIZER_FLAGS} -fsanitize-coverage=trace-pc-guard")
    endif()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SANITIZER_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SANITIZER_FLAGS}")
endif()

## include directories
include_directories(include)

add_subdirectory(src)
add_subdirectory(include)

if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

