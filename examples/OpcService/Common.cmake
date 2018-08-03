#Common CMAKE definitons for 
# where to install
# Get the common definitons at the higher level - BOOST
#
# Common CMAKE definitions
set(CMAKE_CXX_STANDARD 14)
#
set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 

find_package(Boost COMPONENTS REQUIRED system thread program_options)

# add boost to target
if(Boost_FOUND)
    include_directories(${BOOST_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
    add_definitions(${Boost_DEFINITIONS})
    include_directories(${Boost_INCLUDE_DIRS})  
endif()

set(CMAKE_INSTALL_PREFIX ${PIROOT}/usr/local/MRL5/OpcService)
add_definitions( -DUA_LOGLEVEL=100 )
#
# Add the Open62541 includes
include_directories(  ${PIROOT}/usr/local/MRL5 )
#
# list the libraries
find_library(OPEN62541 Open62541Cpp /usr/local/MRL5/lib)
include_directories(${PIROOT}/usr/local/MRL5/include)
include_directories(${PIROOT}/usr/local/MRL5/include/Open62541Cpp)

find_library(OPCSERVICECOMMON OpcServiceCommon ${CMAKE_SOURCE_DIR}/../../OpcServiceCommon)
#
set(WTLIBS -lDWTRuntime  -lDWTData -lwt -lwthttp)
include_directories(${CMAKE_SOURCE_DIR})
#


