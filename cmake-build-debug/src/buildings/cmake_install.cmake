# Install script for directory: /home/espen/ns-3/ns3-wip/src/buildings

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
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

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36.1-buildings-debug.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36.1-buildings-debug.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36.1-buildings-debug.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-buildings-debug.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36.1-buildings-debug.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36.1-buildings-debug.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36.1-buildings-debug.so"
         OLD_RPATH "/home/espen/ns-3/ns3-wip/build/lib:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36.1-buildings-debug.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ns3" TYPE FILE FILES
    "/home/espen/ns-3/ns3-wip/src/buildings/helper/building-allocator.h"
    "/home/espen/ns-3/ns3-wip/src/buildings/helper/building-container.h"
    "/home/espen/ns-3/ns3-wip/src/buildings/helper/building-position-allocator.h"
    "/home/espen/ns-3/ns3-wip/src/buildings/helper/buildings-helper.h"
    "/home/espen/ns-3/ns3-wip/src/buildings/model/building-list.h"
    "/home/espen/ns-3/ns3-wip/src/buildings/model/building.h"
    "/home/espen/ns-3/ns3-wip/src/buildings/model/buildings-channel-condition-model.h"
    "/home/espen/ns-3/ns3-wip/src/buildings/model/buildings-propagation-loss-model.h"
    "/home/espen/ns-3/ns3-wip/src/buildings/model/hybrid-buildings-propagation-loss-model.h"
    "/home/espen/ns-3/ns3-wip/src/buildings/model/itu-r-1238-propagation-loss-model.h"
    "/home/espen/ns-3/ns3-wip/src/buildings/model/mobility-building-info.h"
    "/home/espen/ns-3/ns3-wip/src/buildings/model/oh-buildings-propagation-loss-model.h"
    "/home/espen/ns-3/ns3-wip/src/buildings/model/random-walk-2d-outdoor-mobility-model.h"
    "/home/espen/ns-3/ns3-wip/src/buildings/model/three-gpp-v2v-channel-condition-model.h"
    "/home/espen/ns-3/ns3-wip/build/include/ns3/buildings-module.h"
    )
endif()
