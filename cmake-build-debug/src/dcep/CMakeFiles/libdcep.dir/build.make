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
include src/dcep/CMakeFiles/libdcep.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/dcep/CMakeFiles/libdcep.dir/compiler_depend.make

# Include the progress variables for this target.
include src/dcep/CMakeFiles/libdcep.dir/progress.make

# Include the compile flags for this target's objects.
include src/dcep/CMakeFiles/libdcep.dir/flags.make

# Object files for target libdcep
libdcep_OBJECTS =

# External object files for target libdcep
libdcep_EXTERNAL_OBJECTS = \
"/home/espen/ns-3/ns3-wip/cmake-build-debug/src/dcep/CMakeFiles/libdcep-obj.dir/model/communication.cc.o" \
"/home/espen/ns-3/ns3-wip/cmake-build-debug/src/dcep/CMakeFiles/libdcep-obj.dir/model/placement.cc.o" \
"/home/espen/ns-3/ns3-wip/cmake-build-debug/src/dcep/CMakeFiles/libdcep-obj.dir/model/cep-engine.cc.o" \
"/home/espen/ns-3/ns3-wip/cmake-build-debug/src/dcep/CMakeFiles/libdcep-obj.dir/model/dcep.cc.o" \
"/home/espen/ns-3/ns3-wip/cmake-build-debug/src/dcep/CMakeFiles/libdcep-obj.dir/model/dcep-header.cc.o" \
"/home/espen/ns-3/ns3-wip/cmake-build-debug/src/dcep/CMakeFiles/libdcep-obj.dir/helper/dcep-app-helper.cc.o" \
"/home/espen/ns-3/ns3-wip/cmake-build-debug/src/dcep/CMakeFiles/libdcep-obj.dir/model/utility/CSVIterator.cpp.o" \
"/home/espen/ns-3/ns3-wip/cmake-build-debug/src/dcep/CMakeFiles/libdcep-obj.dir/model/utility/knapsack_solver.cpp.o" \
"/home/espen/ns-3/ns3-wip/cmake-build-debug/src/dcep/CMakeFiles/libdcep-obj.dir/model/utility/utility.cpp.o" \
"/home/espen/ns-3/ns3-wip/cmake-build-debug/src/dcep/CMakeFiles/libdcep-obj.dir/model/dcep-apps/expose-coordinator.cc.o" \
"/home/espen/ns-3/ns3-wip/cmake-build-debug/src/dcep/CMakeFiles/libdcep-obj.dir/model/dcep-apps/expose-wrapper.cc.o" \
"/home/espen/ns-3/ns3-wip/cmake-build-debug/src/dcep/CMakeFiles/libdcep-obj.dir/model/dcep-apps/dataset-manipulator.cc.o"

/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-dcep-debug.so: src/dcep/CMakeFiles/libdcep-obj.dir/model/communication.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-dcep-debug.so: src/dcep/CMakeFiles/libdcep-obj.dir/model/placement.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-dcep-debug.so: src/dcep/CMakeFiles/libdcep-obj.dir/model/cep-engine.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-dcep-debug.so: src/dcep/CMakeFiles/libdcep-obj.dir/model/dcep.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-dcep-debug.so: src/dcep/CMakeFiles/libdcep-obj.dir/model/dcep-header.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-dcep-debug.so: src/dcep/CMakeFiles/libdcep-obj.dir/helper/dcep-app-helper.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-dcep-debug.so: src/dcep/CMakeFiles/libdcep-obj.dir/model/utility/CSVIterator.cpp.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-dcep-debug.so: src/dcep/CMakeFiles/libdcep-obj.dir/model/utility/knapsack_solver.cpp.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-dcep-debug.so: src/dcep/CMakeFiles/libdcep-obj.dir/model/utility/utility.cpp.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-dcep-debug.so: src/dcep/CMakeFiles/libdcep-obj.dir/model/dcep-apps/expose-coordinator.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-dcep-debug.so: src/dcep/CMakeFiles/libdcep-obj.dir/model/dcep-apps/expose-wrapper.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-dcep-debug.so: src/dcep/CMakeFiles/libdcep-obj.dir/model/dcep-apps/dataset-manipulator.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-dcep-debug.so: src/dcep/CMakeFiles/libdcep.dir/build.make
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-dcep-debug.so: /usr/local/lib/libyaml-cpp.a
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-dcep-debug.so: /usr/lib/x86_64-linux-gnu/librt.a
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-dcep-debug.so: /usr/lib/x86_64-linux-gnu/libsqlite3.so
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-dcep-debug.so: src/dcep/CMakeFiles/libdcep.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/espen/ns-3/ns3-wip/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Linking CXX shared library /home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-dcep-debug.so"
	cd /home/espen/ns-3/ns3-wip/cmake-build-debug/src/dcep && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/libdcep.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/dcep/CMakeFiles/libdcep.dir/build: /home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-dcep-debug.so
.PHONY : src/dcep/CMakeFiles/libdcep.dir/build

src/dcep/CMakeFiles/libdcep.dir/clean:
	cd /home/espen/ns-3/ns3-wip/cmake-build-debug/src/dcep && $(CMAKE_COMMAND) -P CMakeFiles/libdcep.dir/cmake_clean.cmake
.PHONY : src/dcep/CMakeFiles/libdcep.dir/clean

src/dcep/CMakeFiles/libdcep.dir/depend:
	cd /home/espen/ns-3/ns3-wip/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/espen/ns-3/ns3-wip /home/espen/ns-3/ns3-wip/src/dcep /home/espen/ns-3/ns3-wip/cmake-build-debug /home/espen/ns-3/ns3-wip/cmake-build-debug/src/dcep /home/espen/ns-3/ns3-wip/cmake-build-debug/src/dcep/CMakeFiles/libdcep.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/dcep/CMakeFiles/libdcep.dir/depend

