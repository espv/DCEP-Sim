# Install script for directory: /home/espen/ns-3/ns3-wip/src/uan

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

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/espen/ns-3/ns3-wip/cmake-cache/src/uan/examples/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36.1-uan-debug.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36.1-uan-debug.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36.1-uan-debug.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-uan-debug.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36.1-uan-debug.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36.1-uan-debug.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36.1-uan-debug.so"
         OLD_RPATH "/home/espen/ns-3/ns3-wip/build/lib:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36.1-uan-debug.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ns3" TYPE FILE FILES
    "/home/espen/ns-3/ns3-wip/src/uan/helper/acoustic-modem-energy-model-helper.h"
    "/home/espen/ns-3/ns3-wip/src/uan/helper/uan-helper.h"
    "/home/espen/ns-3/ns3-wip/src/uan/model/acoustic-modem-energy-model.h"
    "/home/espen/ns-3/ns3-wip/src/uan/model/uan-channel.h"
    "/home/espen/ns-3/ns3-wip/src/uan/model/uan-header-common.h"
    "/home/espen/ns-3/ns3-wip/src/uan/model/uan-header-rc.h"
    "/home/espen/ns-3/ns3-wip/src/uan/model/uan-mac-aloha.h"
    "/home/espen/ns-3/ns3-wip/src/uan/model/uan-mac-cw.h"
    "/home/espen/ns-3/ns3-wip/src/uan/model/uan-mac-rc-gw.h"
    "/home/espen/ns-3/ns3-wip/src/uan/model/uan-mac-rc.h"
    "/home/espen/ns-3/ns3-wip/src/uan/model/uan-mac.h"
    "/home/espen/ns-3/ns3-wip/src/uan/model/uan-net-device.h"
    "/home/espen/ns-3/ns3-wip/src/uan/model/uan-noise-model-default.h"
    "/home/espen/ns-3/ns3-wip/src/uan/model/uan-noise-model.h"
    "/home/espen/ns-3/ns3-wip/src/uan/model/uan-phy-dual.h"
    "/home/espen/ns-3/ns3-wip/src/uan/model/uan-phy-gen.h"
    "/home/espen/ns-3/ns3-wip/src/uan/model/uan-phy.h"
    "/home/espen/ns-3/ns3-wip/src/uan/model/uan-prop-model-ideal.h"
    "/home/espen/ns-3/ns3-wip/src/uan/model/uan-prop-model-thorp.h"
    "/home/espen/ns-3/ns3-wip/src/uan/model/uan-prop-model.h"
    "/home/espen/ns-3/ns3-wip/src/uan/model/uan-transducer-hd.h"
    "/home/espen/ns-3/ns3-wip/src/uan/model/uan-transducer.h"
    "/home/espen/ns-3/ns3-wip/src/uan/model/uan-tx-mode.h"
    "/home/espen/ns-3/ns3-wip/build/include/ns3/uan-module.h"
    )
endif()

