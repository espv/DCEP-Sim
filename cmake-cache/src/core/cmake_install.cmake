# Install script for directory: /home/espen/ns-3/ns3-wip/src/core

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
  include("/home/espen/ns-3/ns3-wip/cmake-cache/src/core/examples/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36.1-core-debug.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36.1-core-debug.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36.1-core-debug.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-core-debug.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36.1-core-debug.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36.1-core-debug.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36.1-core-debug.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ns3" TYPE FILE FILES
    "/home/espen/ns-3/ns3-wip/src/core/model/realtime-simulator-impl.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/wall-clock-synchronizer.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/int64x64-128.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/example-as-test.h"
    "/home/espen/ns-3/ns3-wip/src/core/helper/csv-reader.h"
    "/home/espen/ns-3/ns3-wip/src/core/helper/event-garbage-collector.h"
    "/home/espen/ns-3/ns3-wip/src/core/helper/random-variable-stream-helper.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/abort.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/ascii-file.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/ascii-test.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/assert.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/attribute-accessor-helper.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/attribute-construction-list.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/attribute-container-accessor-helper.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/attribute-container.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/attribute-helper.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/attribute.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/boolean.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/breakpoint.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/build-profile.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/calendar-scheduler.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/callback.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/command-line.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/config.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/default-deleter.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/default-simulator-impl.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/deprecated.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/des-metrics.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/double.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/empty.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/enum.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/event-id.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/event-impl.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/fatal-error.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/fatal-impl.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/global-value.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/hash-fnv.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/hash-function.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/hash-murmur3.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/hash.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/heap-scheduler.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/int-to-type.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/int64x64-double.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/int64x64.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/integer.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/length.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/list-scheduler.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/log-macros-disabled.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/log-macros-enabled.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/log.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/make-event.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/map-scheduler.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/math.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/names.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/node-printer.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/nstime.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/object-base.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/object-factory.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/object-map.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/object-ptr-container.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/object-vector.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/object.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/pair.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/pointer.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/priority-queue-scheduler.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/ptr.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/random-variable-stream.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/ref-count-base.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/rng-seed-manager.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/rng-stream.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/scheduler.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/show-progress.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/simple-ref-count.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/simulation-singleton.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/simulator-impl.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/simulator.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/singleton.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/string.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/synchronizer.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/system-path.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/system-wall-clock-ms.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/system-wall-clock-timestamp.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/test.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/time-printer.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/timer-impl.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/timer.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/trace-source-accessor.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/traced-callback.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/traced-value.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/trickle-timer.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/tuple.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/type-id.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/type-name.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/type-traits.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/uinteger.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/unix-fd-reader.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/unused.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/valgrind.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/vector.h"
    "/home/espen/ns-3/ns3-wip/src/core/model/watchdog.h"
    "/home/espen/ns-3/ns3-wip/build/include/ns3/config-store-config.h"
    "/home/espen/ns-3/ns3-wip/build/include/ns3/core-config.h"
    "/home/espen/ns-3/ns3-wip/build/include/ns3/core-module.h"
    )
endif()

