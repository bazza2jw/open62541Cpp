# Common CMAKE definitions
IF( DEFINED CROSSBUILD )
message ("Raspberry Pi Cross Build:${PIROOT}")
set(CMAKE_INSTALL_PREFIX ${PIROOT}/usr/local)
find_library(OPEN62541 open62541 ${PIROOT}/usr/local/lib)
ELSE( DEFINED CROSSBUILD )
message("Native Build")
set(CMAKE_INSTALL_PREFIX /usr/local)
find_library(OPEN62541 open62541 ${CMAKE_INSTALL_PREFIX}/lib)
ENDIF( DEFINED CROSSBUILD )

# the open62541 C library must have been installed
#
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 

if(NOT Boost_FOUND)
    find_package(Boost COMPONENTS REQUIRED system thread)
endif()

# add boost to target
if(Boost_FOUND)
    link_directories(${Boost_LIBRARY_DIRS})
    add_definitions(${Boost_DEFINITIONS})
    include_directories(${Boost_INCLUDE_DIRS})  
endif()

