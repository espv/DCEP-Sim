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
CMAKE_COMMAND = /home/espen/Editors/clion-2021.2.3/bin/cmake/linux/x64/bin/cmake

# The command to remove a file.
RM = /home/espen/Editors/clion-2021.2.3/bin/cmake/linux/x64/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/espen/ns-3/ns3-wip

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/espen/ns-3/ns3-wip/cmake-build-debug

# Include any dependencies generated for this target.
include src/bridge/CMakeFiles/libbridge.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/bridge/CMakeFiles/libbridge.dir/compiler_depend.make

# Include the progress variables for this target.
include src/bridge/CMakeFiles/libbridge.dir/progress.make

# Include the compile flags for this target's objects.
include src/bridge/CMakeFiles/libbridge.dir/flags.make

# Object files for target libbridge
libbridge_OBJECTS =

# External object files for target libbridge
libbridge_EXTERNAL_OBJECTS = \
"/home/espen/ns-3/ns3-wip/cmake-build-debug/src/bridge/CMakeFiles/libbridge-obj.dir/helper/bridge-helper.cc.o" \
"/home/espen/ns-3/ns3-wip/cmake-build-debug/src/bridge/CMakeFiles/libbridge-obj.dir/model/bridge-channel.cc.o" \
"/home/espen/ns-3/ns3-wip/cmake-build-debug/src/bridge/CMakeFiles/libbridge-obj.dir/model/bridge-net-device.cc.o"

/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-bridge-debug.so: src/bridge/CMakeFiles/libbridge-obj.dir/helper/bridge-helper.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-bridge-debug.so: src/bridge/CMakeFiles/libbridge-obj.dir/model/bridge-channel.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-bridge-debug.so: src/bridge/CMakeFiles/libbridge-obj.dir/model/bridge-net-device.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-bridge-debug.so: src/bridge/CMakeFiles/libbridge.dir/build.make
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-bridge-debug.so: /usr/lib/x86_64-linux-gnu/librt.a
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-bridge-debug.so: /usr/lib/x86_64-linux-gnu/libsqlite3.so
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-bridge-debug.so: src/bridge/CMakeFiles/libbridge.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/espen/ns-3/ns3-wip/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Linking CXX shared library /home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-bridge-debug.so"
	cd /home/espen/ns-3/ns3-wip/cmake-build-debug/src/bridge && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/libbridge.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/bridge/CMakeFiles/libbridge.dir/build: /home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-bridge-debug.so
.PHONY : src/bridge/CMakeFiles/libbridge.dir/build

src/bridge/CMakeFiles/libbridge.dir/clean:
	cd /home/espen/ns-3/ns3-wip/cmake-build-debug/src/bridge && $(CMAKE_COMMAND) -P CMakeFiles/libbridge.dir/cmake_clean.cmake
.PHONY : src/bridge/CMakeFiles/libbridge.dir/clean

src/bridge/CMakeFiles/libbridge.dir/depend:
	cd /home/espen/ns-3/ns3-wip/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/espen/ns-3/ns3-wip /home/espen/ns-3/ns3-wip/src/bridge /home/espen/ns-3/ns3-wip/cmake-build-debug /home/espen/ns-3/ns3-wip/cmake-build-debug/src/bridge /home/espen/ns-3/ns3-wip/cmake-build-debug/src/bridge/CMakeFiles/libbridge.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/bridge/CMakeFiles/libbridge.dir/depend

