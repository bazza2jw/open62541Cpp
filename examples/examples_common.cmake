cmake_minimum_required (VERSION 3.9)
project (${APPNAME})

add_definitions( -DUA_LOGLEVEL=100 )

add_executable(${APPNAME} ${SOURCES})
target_link_libraries (${APPNAME} open62541cpp)

# add boost to target
if(Boost_FOUND)
    target_link_libraries(${APPNAME} ${Boost_LIBRARIES})
endif()
