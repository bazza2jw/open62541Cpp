cmake_minimum_required(VERSION 3.11)
project(TestServer)
# Build Test Server
set(APPNAME TestServer)

# Source code
set(SOURCES
        main.cpp
        testcontext.cpp
        testmethod.cpp
        testobject.cpp
        )

include(../examples_common.cmake)
