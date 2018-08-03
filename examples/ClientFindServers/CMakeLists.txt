# Build Test Server
set(APPNAME ClientFindServers)
cmake_minimum_required (VERSION 3.9)
project (${APPNAME})
set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_INSTALL_PREFIX /usr/local)
add_definitions( -DUA_LOGLEVEL=100 )
# Source code
set(SOURCES main.cpp)
#
# Add Boost
#
set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost COMPONENTS REQUIRED system thread)
include_directories(${Boost_INCLUDE_DIR})
#
# Add the Open62541 includes
include_directories( ../../include )
#
# Add Open62541 libraries
find_library(OPEN62541 Open62541Cpp /usr/local/MRL5/lib)
add_executable(${APPNAME} ${SOURCES} )
target_link_libraries (${APPNAME}  ${OPEN62541})
# add boost to target
if(Boost_FOUND)
    include_directories(${BOOST_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
    add_definitions(${Boost_DEFINITIONS})
    include_directories(${Boost_INCLUDE_DIRS})  
    target_link_libraries(${APPNAME} ${Boost_LIBRARIES})
endif()


