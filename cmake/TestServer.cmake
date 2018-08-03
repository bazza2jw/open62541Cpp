###############################################################################
cmake_minimum_required (VERSION 2.6)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INSTALL_PREFIX /usr/local)
#
project (TestServer)
set(sources TestServer/main.cpp)
add_executable(TestServer ${sources})



# just for example add some compiler flags
target_compile_options(example PUBLIC -std=c++1y -Wall -Wfloat-conversion)

# this lets me include files relative to the root src dir with a <> pair
target_include_directories(example PUBLIC src/main)

# this copies all resource files in the build directory
# we need this, because we want to work with paths relative to the executable
file(COPY ${data} DESTINATION resources)

###############################################################################
## dependencies ###############################################################
###############################################################################

# this defines the variables Boost_LIBRARIES that contain all library names
# that we need to link to
find_package(Boost 1.36.0 COMPONENTS filesystem system REQUIRED)

target_link_libraries(example PUBLIC
  ${Boost_LIBRARIES}
  # here you can add any library dependencies
)
