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
include src/point-to-point/CMakeFiles/libpoint-to-point.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/point-to-point/CMakeFiles/libpoint-to-point.dir/compiler_depend.make

# Include the progress variables for this target.
include src/point-to-point/CMakeFiles/libpoint-to-point.dir/progress.make

# Include the compile flags for this target's objects.
include src/point-to-point/CMakeFiles/libpoint-to-point.dir/flags.make

# Object files for target libpoint-to-point
libpoint__to__point_OBJECTS =

# External object files for target libpoint-to-point
libpoint__to__point_EXTERNAL_OBJECTS = \
"/home/espen/ns-3/ns3-wip/cmake-build-debug/src/point-to-point/CMakeFiles/libpoint-to-point-obj.dir/helper/point-to-point-helper.cc.o" \
"/home/espen/ns-3/ns3-wip/cmake-build-debug/src/point-to-point/CMakeFiles/libpoint-to-point-obj.dir/model/point-to-point-channel.cc.o" \
"/home/espen/ns-3/ns3-wip/cmake-build-debug/src/point-to-point/CMakeFiles/libpoint-to-point-obj.dir/model/point-to-point-net-device.cc.o" \
"/home/espen/ns-3/ns3-wip/cmake-build-debug/src/point-to-point/CMakeFiles/libpoint-to-point-obj.dir/model/ppp-header.cc.o"

/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-point-to-point-debug.so: src/point-to-point/CMakeFiles/libpoint-to-point-obj.dir/helper/point-to-point-helper.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-point-to-point-debug.so: src/point-to-point/CMakeFiles/libpoint-to-point-obj.dir/model/point-to-point-channel.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-point-to-point-debug.so: src/point-to-point/CMakeFiles/libpoint-to-point-obj.dir/model/point-to-point-net-device.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-point-to-point-debug.so: src/point-to-point/CMakeFiles/libpoint-to-point-obj.dir/model/ppp-header.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-point-to-point-debug.so: src/point-to-point/CMakeFiles/libpoint-to-point.dir/build.make
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-point-to-point-debug.so: /usr/lib/x86_64-linux-gnu/librt.a
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-point-to-point-debug.so: /usr/lib/x86_64-linux-gnu/libsqlite3.so
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-point-to-point-debug.so: src/point-to-point/CMakeFiles/libpoint-to-point.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/espen/ns-3/ns3-wip/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Linking CXX shared library /home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-point-to-point-debug.so"
	cd /home/espen/ns-3/ns3-wip/cmake-build-debug/src/point-to-point && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/libpoint-to-point.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/point-to-point/CMakeFiles/libpoint-to-point.dir/build: /home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-point-to-point-debug.so
.PHONY : src/point-to-point/CMakeFiles/libpoint-to-point.dir/build

src/point-to-point/CMakeFiles/libpoint-to-point.dir/clean:
	cd /home/espen/ns-3/ns3-wip/cmake-build-debug/src/point-to-point && $(CMAKE_COMMAND) -P CMakeFiles/libpoint-to-point.dir/cmake_clean.cmake
.PHONY : src/point-to-point/CMakeFiles/libpoint-to-point.dir/clean

src/point-to-point/CMakeFiles/libpoint-to-point.dir/depend:
	cd /home/espen/ns-3/ns3-wip/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/espen/ns-3/ns3-wip /home/espen/ns-3/ns3-wip/src/point-to-point /home/espen/ns-3/ns3-wip/cmake-build-debug /home/espen/ns-3/ns3-wip/cmake-build-debug/src/point-to-point /home/espen/ns-3/ns3-wip/cmake-build-debug/src/point-to-point/CMakeFiles/libpoint-to-point.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/point-to-point/CMakeFiles/libpoint-to-point.dir/depend

