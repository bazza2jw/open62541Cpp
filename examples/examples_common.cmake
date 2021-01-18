cmake_minimum_required (VERSION 3.9)
project(${APPNAME})

# Add Open62541 libraries + all its dependecies (boost)
add_executable(${APPNAME} ${SOURCES})
target_link_libraries(${APPNAME} PRIVATE open62541cpp)
include(../../Common.cmake)

set_build_system_option(${APPNAME})
