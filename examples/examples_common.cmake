cmake_minimum_required (VERSION 3.9)
project (${APPNAME})

#set(CMAKE_INSTALL_PREFIX /usr/local)
add_definitions( -DUA_LOGLEVEL=100 )

# Add Boost
include(../../Common.cmake)

# Add the Open62541 includes
include_directories( ../../include )

# Add Open62541 libraries
add_executable(${APPNAME} ${SOURCES})
target_link_libraries (${APPNAME} open62541cpp)

# add boost to target
if(Boost_FOUND)
    target_link_libraries(${APPNAME} ${Boost_LIBRARIES})
endif()

install(TARGETS ${APPNAME}
        RUNTIME DESTINATION bin
        )
