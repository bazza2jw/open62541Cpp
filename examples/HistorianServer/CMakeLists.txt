cmake_minimum_required(VERSION 3.11)
# Build Test Server
set(APPNAME TestHistorianServer)

# Source code
set(SOURCES
        main.cpp
        testcontext.cpp
        testmethod.cpp
        testobject.cpp
        )

include(../examples_common.cmake)
