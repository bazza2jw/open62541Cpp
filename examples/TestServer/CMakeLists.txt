cmake_minimum_required(VERSION 3.11)

include(../examples_common.cmake)
add_example("TestServer"
    main.cpp
    testcontext.cpp
    testmethod.cpp
    testobject.cpp
    testcontext.h
    testmethod.h
    testobject.h
    )
