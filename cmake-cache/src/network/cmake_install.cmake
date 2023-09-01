# Install script for directory: /home/espen/ns-3/ns3-wip/src/network

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
  include("/home/espen/ns-3/ns3-wip/cmake-cache/src/network/examples/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36.1-network-debug.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36.1-network-debug.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36.1-network-debug.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-network-debug.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36.1-network-debug.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36.1-network-debug.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36.1-network-debug.so"
         OLD_RPATH "/home/espen/ns-3/ns3-wip/build/lib:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36.1-network-debug.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ns3" TYPE FILE FILES
    "/home/espen/ns-3/ns3-wip/src/network/helper/application-container.h"
    "/home/espen/ns-3/ns3-wip/src/network/helper/delay-jitter-estimation.h"
    "/home/espen/ns-3/ns3-wip/src/network/helper/net-device-container.h"
    "/home/espen/ns-3/ns3-wip/src/network/helper/node-container.h"
    "/home/espen/ns-3/ns3-wip/src/network/helper/packet-socket-helper.h"
    "/home/espen/ns-3/ns3-wip/src/network/helper/simple-net-device-helper.h"
    "/home/espen/ns-3/ns3-wip/src/network/helper/trace-helper.h"
    "/home/espen/ns-3/ns3-wip/src/network/model/address.h"
    "/home/espen/ns-3/ns3-wip/src/network/model/application.h"
    "/home/espen/ns-3/ns3-wip/src/network/model/buffer.h"
    "/home/espen/ns-3/ns3-wip/src/network/model/byte-tag-list.h"
    "/home/espen/ns-3/ns3-wip/src/network/model/channel-list.h"
    "/home/espen/ns-3/ns3-wip/src/network/model/channel.h"
    "/home/espen/ns-3/ns3-wip/src/network/model/chunk.h"
    "/home/espen/ns-3/ns3-wip/src/network/model/header.h"
    "/home/espen/ns-3/ns3-wip/src/network/model/net-device.h"
    "/home/espen/ns-3/ns3-wip/src/network/model/nix-vector.h"
    "/home/espen/ns-3/ns3-wip/src/network/model/node-list.h"
    "/home/espen/ns-3/ns3-wip/src/network/model/node.h"
    "/home/espen/ns-3/ns3-wip/src/network/model/packet-metadata.h"
    "/home/espen/ns-3/ns3-wip/src/network/model/packet-tag-list.h"
    "/home/espen/ns-3/ns3-wip/src/network/model/packet.h"
    "/home/espen/ns-3/ns3-wip/src/network/model/socket-factory.h"
    "/home/espen/ns-3/ns3-wip/src/network/model/socket.h"
    "/home/espen/ns-3/ns3-wip/src/network/model/tag-buffer.h"
    "/home/espen/ns-3/ns3-wip/src/network/model/tag.h"
    "/home/espen/ns-3/ns3-wip/src/network/model/trailer.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/address-utils.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/bit-deserializer.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/bit-serializer.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/crc32.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/data-rate.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/drop-tail-queue.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/dynamic-queue-limits.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/error-channel.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/error-model.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/ethernet-header.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/ethernet-trailer.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/flow-id-tag.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/generic-phy.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/inet-socket-address.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/inet6-socket-address.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/ipv4-address.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/ipv6-address.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/llc-snap-header.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/lollipop-counter.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/mac16-address.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/mac48-address.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/mac64-address.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/mac8-address.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/net-device-queue-interface.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/output-stream-wrapper.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/packet-burst.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/packet-data-calculators.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/packet-probe.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/packet-socket-address.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/packet-socket-client.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/packet-socket-factory.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/packet-socket-server.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/packet-socket.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/packetbb.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/pcap-file-wrapper.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/pcap-file.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/pcap-test.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/queue-item.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/queue-limits.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/queue-size.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/queue.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/radiotap-header.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/sequence-number.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/simple-channel.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/simple-net-device.h"
    "/home/espen/ns-3/ns3-wip/src/network/utils/sll-header.h"
    "/home/espen/ns-3/ns3-wip/build/include/ns3/network-module.h"
    )
endif()

