cmake_minimum_required(VERSION 3.5)
project(OpcService)
include(Common.cmake)
#
# Build the library
file(GLOB LIB_SOURCES OpcServiceCommon/*.cpp)
#
add_library(OpcServiceCommon SHARED ${LIB_SOURCES})
#
# 2. Link...
target_link_libraries(OpcServiceCommon ${WTLIBS})
target_link_libraries(OpcServiceCommon PRIVATE open62541cpp)
#
# now add each of the server builds 
#
foreach(SERVER_EXE Simulator)
  set(SERVER_DIR ${CMAKE_SOURCE_DIR}/Servers/${SERVER_EXE})
  file(GLOB SOURCES ${SERVER_DIR}/*.cpp)

  ## Specify a name for the generated executable file
  ## 1. Compile...
  add_executable(${SERVER_EXE}  ${SOURCES})

  ## 2. Link...
  target_link_libraries(${SERVER_EXE} ${WTLIBS})
  target_link_libraries(${SERVER_EXE} PRIVATE open62541cpp)
  target_link_libraries(${SERVER_EXE} OpcServiceCommon)

  ## 3. Install...
  install(TARGETS ${SERVER_EXE} RUNTIME DESTINATION bin)
endforeach(SERVER_EXE)
#
#
## 3. Install...
install(TARGETS OpcServiceCommon DESTINATION bin)


