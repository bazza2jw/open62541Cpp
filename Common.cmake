# Common CMAKE definitions

#Find and add Boost libraries
if(NOT Boost_FOUND)
    set(Boost_USE_STATIC_LIBS OFF)
    set(Boost_USE_MULTITHREADED ON)
    set(Boost_USE_STATIC_RUNTIME OFF)

    if(DEFINED ENV{OXSAS_THIRD_PARTY_PATH})
        set(BOOST_ROOT        "$ENV{OXSAS_THIRD_PARTY_PATH}/Boost/boost_1_69_vc140")
        set(BOOST_LIBRARYDIR  "${BOOST_ROOT}/dll")

        message("3rdParty folder path definded:     " $ENV{OXSAS_THIRD_PARTY_PATH})
        message("Taking boost from 3rdParty folder: " ${BOOST_ROOT})
    endif()
    
    # set(Boost_DEBUG 1) # debug FindBoost
    add_definitions(-DBOOST_ALL_NO_LIB)
    # add_definitions(-DBOOST_ALL_DYN_LINK)
    find_package(Boost COMPONENTS REQUIRED system thread)
endif()

# add boost to target
if(Boost_FOUND)
    link_directories(${Boost_LIBRARY_DIRS})
    add_definitions(${Boost_DEFINITIONS})
    include_directories(${Boost_INCLUDE_DIRS})
endif()
