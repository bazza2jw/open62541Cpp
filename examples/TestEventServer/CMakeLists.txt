cmake_minimum_required(VERSION 3.11)
# Build Test Server
set(APPNAME TestEventServer)

# Source code
set(SOURCES
        main.cpp
        testmethod.cpp
        )

include(../examples_common.cmake)
