#
# Common CMAKE definitions
set(CMAKE_CXX_STANDARD 14)
#
set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 

find_package(Boost COMPONENTS REQUIRED system thread)

# add boost to target
if(Boost_FOUND)
    include_directories(${BOOST_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
    add_definitions(${Boost_DEFINITIONS})
    include_directories(${Boost_INCLUDE_DIRS})  
endif()

