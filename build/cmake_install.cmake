# Install script for directory: /home/barry/Work/Development/Projects/MRL/Open62541Cpp

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local/MRL5")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libOpen62541Cpp.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libOpen62541Cpp.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libOpen62541Cpp.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/barry/Work/Development/Projects/MRL/Open62541Cpp/build/libOpen62541Cpp.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libOpen62541Cpp.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libOpen62541Cpp.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libOpen62541Cpp.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/Open62541Cpp" TYPE FILE FILES
    "/home/barry/Work/Development/Projects/MRL/Open62541Cpp/include/open62541client.h"
    "/home/barry/Work/Development/Projects/MRL/Open62541Cpp/include/open62541objects.h"
    "/home/barry/Work/Development/Projects/MRL/Open62541Cpp/include/open62541.h"
    "/home/barry/Work/Development/Projects/MRL/Open62541Cpp/include/open62541server.h"
    "/home/barry/Work/Development/Projects/MRL/Open62541Cpp/include/propertytree.h"
    "/home/barry/Work/Development/Projects/MRL/Open62541Cpp/include/clientcache.h"
    "/home/barry/Work/Development/Projects/MRL/Open62541Cpp/include/clientcachethread.h"
    "/home/barry/Work/Development/Projects/MRL/Open62541Cpp/include/nodecontext.h"
    "/home/barry/Work/Development/Projects/MRL/Open62541Cpp/include/servermethod.h"
    "/home/barry/Work/Development/Projects/MRL/Open62541Cpp/include/serverrepeatedcallback.h"
    "/home/barry/Work/Development/Projects/MRL/Open62541Cpp/include/serverobjecttype.h"
    "/home/barry/Work/Development/Projects/MRL/Open62541Cpp/include/serverbrowser.h"
    "/home/barry/Work/Development/Projects/MRL/Open62541Cpp/include/servernodetree.h"
    "/home/barry/Work/Development/Projects/MRL/Open62541Cpp/include/clientnodetree.h"
    "/home/barry/Work/Development/Projects/MRL/Open62541Cpp/include/clientbrowser.h"
    "/home/barry/Work/Development/Projects/MRL/Open62541Cpp/include/monitoreditem.h"
    "/home/barry/Work/Development/Projects/MRL/Open62541Cpp/include/clientsubscription.h"
    "/home/barry/Work/Development/Projects/MRL/Open62541Cpp/include/trace.h"
    "/home/barry/Work/Development/Projects/MRL/Open62541Cpp/include/discoveryserver.h"
    "/home/barry/Work/Development/Projects/MRL/Open62541Cpp/include/monitoreditem.h"
    "/home/barry/Work/Development/Projects/MRL/Open62541Cpp/include/clientsubscription.h"
    "/home/barry/Work/Development/Projects/MRL/Open62541Cpp/include/trace.h"
    "/home/barry/Work/Development/Projects/MRL/Open62541Cpp/include/discoveryserver.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/barry/Work/Development/Projects/MRL/Open62541Cpp/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
