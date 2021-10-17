cmake_minimum_required (VERSION 3.9)
project (${APPNAME})
find_package(Boost REQUIRED COMPONENTS system thread program_options)
add_definitions( -DUA_LOGLEVEL=100 )

add_executable(${APPNAME} ${SOURCES})
target_link_libraries (${APPNAME} open62541cpp open62541)

# add boost to target
if(Boost_FOUND)
    target_link_libraries(${APPNAME} ${Boost_LIBRARIES})
endif()
