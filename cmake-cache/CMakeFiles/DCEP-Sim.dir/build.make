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

# Utility rule file for DCEP-Sim.

# Include any custom commands dependencies for this target.
include CMakeFiles/DCEP-Sim.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/DCEP-Sim.dir/progress.make

DCEP-Sim: CMakeFiles/DCEP-Sim.dir/build.make
.PHONY : DCEP-Sim

# Rule to build all files generated by this target.
CMakeFiles/DCEP-Sim.dir/build: DCEP-Sim
.PHONY : CMakeFiles/DCEP-Sim.dir/build

CMakeFiles/DCEP-Sim.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/DCEP-Sim.dir/cmake_clean.cmake
.PHONY : CMakeFiles/DCEP-Sim.dir/clean

CMakeFiles/DCEP-Sim.dir/depend:
	cd /home/espen/ns-3/ns3-wip/cmake-cache && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/espen/ns-3/ns3-wip /home/espen/ns-3/ns3-wip /home/espen/ns-3/ns3-wip/cmake-cache /home/espen/ns-3/ns3-wip/cmake-cache /home/espen/ns-3/ns3-wip/cmake-cache/CMakeFiles/DCEP-Sim.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/DCEP-Sim.dir/depend

