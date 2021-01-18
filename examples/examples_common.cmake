cmake_minimum_required (VERSION 3.9)

# Create an example project with the given name and sources
# The sources are a list of file name separated by space
# usage 1: add_example(name_of_your_project main.cpp main.h)
# usage 2: set(SOURCES main.cpp main.h)
#          add_example(name_of_your_project ${SOURCES})
function(add_example ex_target)
    project(       ${ex_target})
    add_executable(${ex_target} ${ARGN}) # ${ARGN} expand the arguments after ex_target to a list
    
    # Add Open62541 libraries + all its dependecies (boost)
    target_link_libraries(${ex_target} PRIVATE open62541cpp)

    # force output directory to build/bin
    set_target_properties(${ex_target}
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        )

    include(../../Common.cmake)
    set_build_system_option(${ex_target})
endfunction()