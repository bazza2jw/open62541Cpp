# Common CMAKE definitions

# Find and add Boost libraries
function(add_boost_lib target)
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
    target_compile_definitions(${target} PUBLIC -DBOOST_ALL_NO_LIB)
    find_package(Boost COMPONENTS REQUIRED system thread)
  endif()

  # add boost to target
  if(Boost_FOUND)
    target_link_directories(   ${target} PUBLIC ${Boost_LIBRARY_DIRS})
    target_compile_definitions(${target} PUBLIC ${Boost_DEFINITIONS})
    target_include_directories(${target} PUBLIC ${Boost_INCLUDE_DIRS})
    target_link_libraries(     ${target} PUBLIC ${Boost_LIBRARIES})
  else()
    message(FATAL_ERROR "Boost not found")
  endif()
endfunction()

# Set MSVC options and fixes the default WIN32 lib
function(set_build_system_option target)
  if (WIN32)
    # Replace: kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib
    #      by: ws2_32.lib   the only lib needed.
    set(CMAKE_CXX_STANDARD_LIBRARIES "ws2_32.lib" PARENT_SCOPE)
  endif()

  if(MSVC)
    target_compile_options(${target} PRIVATE /W3 /WX-)
  else()
    target_compile_options(${target} PRIVATE -W3 -Wextra -pedantic)
  endif()
endfunction()