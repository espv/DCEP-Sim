# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.25

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/espen/ns-3/ns3-wip

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/espen/ns-3/ns3-wip/cmake-cache

# Include any dependencies generated for this target.
include src/wave/CMakeFiles/libwave.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/wave/CMakeFiles/libwave.dir/compiler_depend.make

# Include the progress variables for this target.
include src/wave/CMakeFiles/libwave.dir/progress.make

# Include the compile flags for this target's objects.
include src/wave/CMakeFiles/libwave.dir/flags.make

# Object files for target libwave
libwave_OBJECTS =

# External object files for target libwave
libwave_EXTERNAL_OBJECTS = \
"/home/espen/ns-3/ns3-wip/cmake-cache/src/wave/CMakeFiles/libwave-obj.dir/helper/wave-bsm-helper.cc.o" \
"/home/espen/ns-3/ns3-wip/cmake-cache/src/wave/CMakeFiles/libwave-obj.dir/helper/wave-bsm-stats.cc.o" \
"/home/espen/ns-3/ns3-wip/cmake-cache/src/wave/CMakeFiles/libwave-obj.dir/helper/wave-helper.cc.o" \
"/home/espen/ns-3/ns3-wip/cmake-cache/src/wave/CMakeFiles/libwave-obj.dir/helper/wave-mac-helper.cc.o" \
"/home/espen/ns-3/ns3-wip/cmake-cache/src/wave/CMakeFiles/libwave-obj.dir/helper/wifi-80211p-helper.cc.o" \
"/home/espen/ns-3/ns3-wip/cmake-cache/src/wave/CMakeFiles/libwave-obj.dir/model/bsm-application.cc.o" \
"/home/espen/ns-3/ns3-wip/cmake-cache/src/wave/CMakeFiles/libwave-obj.dir/model/channel-coordinator.cc.o" \
"/home/espen/ns-3/ns3-wip/cmake-cache/src/wave/CMakeFiles/libwave-obj.dir/model/channel-manager.cc.o" \
"/home/espen/ns-3/ns3-wip/cmake-cache/src/wave/CMakeFiles/libwave-obj.dir/model/channel-scheduler.cc.o" \
"/home/espen/ns-3/ns3-wip/cmake-cache/src/wave/CMakeFiles/libwave-obj.dir/model/default-channel-scheduler.cc.o" \
"/home/espen/ns-3/ns3-wip/cmake-cache/src/wave/CMakeFiles/libwave-obj.dir/model/higher-tx-tag.cc.o" \
"/home/espen/ns-3/ns3-wip/cmake-cache/src/wave/CMakeFiles/libwave-obj.dir/model/ocb-wifi-mac.cc.o" \
"/home/espen/ns-3/ns3-wip/cmake-cache/src/wave/CMakeFiles/libwave-obj.dir/model/vendor-specific-action.cc.o" \
"/home/espen/ns-3/ns3-wip/cmake-cache/src/wave/CMakeFiles/libwave-obj.dir/model/vsa-manager.cc.o" \
"/home/espen/ns-3/ns3-wip/cmake-cache/src/wave/CMakeFiles/libwave-obj.dir/model/wave-frame-exchange-manager.cc.o" \
"/home/espen/ns-3/ns3-wip/cmake-cache/src/wave/CMakeFiles/libwave-obj.dir/model/wave-net-device.cc.o"

/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-wave-debug.so: src/wave/CMakeFiles/libwave-obj.dir/helper/wave-bsm-helper.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-wave-debug.so: src/wave/CMakeFiles/libwave-obj.dir/helper/wave-bsm-stats.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-wave-debug.so: src/wave/CMakeFiles/libwave-obj.dir/helper/wave-helper.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-wave-debug.so: src/wave/CMakeFiles/libwave-obj.dir/helper/wave-mac-helper.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-wave-debug.so: src/wave/CMakeFiles/libwave-obj.dir/helper/wifi-80211p-helper.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-wave-debug.so: src/wave/CMakeFiles/libwave-obj.dir/model/bsm-application.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-wave-debug.so: src/wave/CMakeFiles/libwave-obj.dir/model/channel-coordinator.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-wave-debug.so: src/wave/CMakeFiles/libwave-obj.dir/model/channel-manager.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-wave-debug.so: src/wave/CMakeFiles/libwave-obj.dir/model/channel-scheduler.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-wave-debug.so: src/wave/CMakeFiles/libwave-obj.dir/model/default-channel-scheduler.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-wave-debug.so: src/wave/CMakeFiles/libwave-obj.dir/model/higher-tx-tag.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-wave-debug.so: src/wave/CMakeFiles/libwave-obj.dir/model/ocb-wifi-mac.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-wave-debug.so: src/wave/CMakeFiles/libwave-obj.dir/model/vendor-specific-action.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-wave-debug.so: src/wave/CMakeFiles/libwave-obj.dir/model/vsa-manager.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-wave-debug.so: src/wave/CMakeFiles/libwave-obj.dir/model/wave-frame-exchange-manager.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-wave-debug.so: src/wave/CMakeFiles/libwave-obj.dir/model/wave-net-device.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-wave-debug.so: src/wave/CMakeFiles/libwave.dir/build.make
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-wave-debug.so: /usr/lib/x86_64-linux-gnu/librt.a
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-wave-debug.so: /usr/lib/x86_64-linux-gnu/libsqlite3.so
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-wave-debug.so: /usr/lib/x86_64-linux-gnu/libxml2.so
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-wave-debug.so: src/wave/CMakeFiles/libwave.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/espen/ns-3/ns3-wip/cmake-cache/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Linking CXX shared library /home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-wave-debug.so"
	cd /home/espen/ns-3/ns3-wip/cmake-cache/src/wave && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/libwave.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/wave/CMakeFiles/libwave.dir/build: /home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-wave-debug.so
.PHONY : src/wave/CMakeFiles/libwave.dir/build

src/wave/CMakeFiles/libwave.dir/clean:
	cd /home/espen/ns-3/ns3-wip/cmake-cache/src/wave && $(CMAKE_COMMAND) -P CMakeFiles/libwave.dir/cmake_clean.cmake
.PHONY : src/wave/CMakeFiles/libwave.dir/clean

src/wave/CMakeFiles/libwave.dir/depend:
	cd /home/espen/ns-3/ns3-wip/cmake-cache && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/espen/ns-3/ns3-wip /home/espen/ns-3/ns3-wip/src/wave /home/espen/ns-3/ns3-wip/cmake-cache /home/espen/ns-3/ns3-wip/cmake-cache/src/wave /home/espen/ns-3/ns3-wip/cmake-cache/src/wave/CMakeFiles/libwave.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/wave/CMakeFiles/libwave.dir/depend

