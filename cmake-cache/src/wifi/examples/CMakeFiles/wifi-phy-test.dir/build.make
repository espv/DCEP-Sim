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
include src/wifi/examples/CMakeFiles/wifi-phy-test.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/wifi/examples/CMakeFiles/wifi-phy-test.dir/compiler_depend.make

# Include the progress variables for this target.
include src/wifi/examples/CMakeFiles/wifi-phy-test.dir/progress.make

# Include the compile flags for this target's objects.
include src/wifi/examples/CMakeFiles/wifi-phy-test.dir/flags.make

src/wifi/examples/CMakeFiles/wifi-phy-test.dir/wifi-phy-test.cc.o: src/wifi/examples/CMakeFiles/wifi-phy-test.dir/flags.make
src/wifi/examples/CMakeFiles/wifi-phy-test.dir/wifi-phy-test.cc.o: /home/espen/ns-3/ns3-wip/src/wifi/examples/wifi-phy-test.cc
src/wifi/examples/CMakeFiles/wifi-phy-test.dir/wifi-phy-test.cc.o: CMakeFiles/stdlib_pch_exec.dir/cmake_pch.hxx
src/wifi/examples/CMakeFiles/wifi-phy-test.dir/wifi-phy-test.cc.o: CMakeFiles/stdlib_pch_exec.dir/cmake_pch.hxx.gch
src/wifi/examples/CMakeFiles/wifi-phy-test.dir/wifi-phy-test.cc.o: src/wifi/examples/CMakeFiles/wifi-phy-test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/espen/ns-3/ns3-wip/cmake-cache/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/wifi/examples/CMakeFiles/wifi-phy-test.dir/wifi-phy-test.cc.o"
	cd /home/espen/ns-3/ns3-wip/cmake-cache/src/wifi/examples && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -Winvalid-pch -include /home/espen/ns-3/ns3-wip/cmake-cache/CMakeFiles/stdlib_pch_exec.dir/cmake_pch.hxx -MD -MT src/wifi/examples/CMakeFiles/wifi-phy-test.dir/wifi-phy-test.cc.o -MF CMakeFiles/wifi-phy-test.dir/wifi-phy-test.cc.o.d -o CMakeFiles/wifi-phy-test.dir/wifi-phy-test.cc.o -c /home/espen/ns-3/ns3-wip/src/wifi/examples/wifi-phy-test.cc

src/wifi/examples/CMakeFiles/wifi-phy-test.dir/wifi-phy-test.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/wifi-phy-test.dir/wifi-phy-test.cc.i"
	cd /home/espen/ns-3/ns3-wip/cmake-cache/src/wifi/examples && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -Winvalid-pch -include /home/espen/ns-3/ns3-wip/cmake-cache/CMakeFiles/stdlib_pch_exec.dir/cmake_pch.hxx -E /home/espen/ns-3/ns3-wip/src/wifi/examples/wifi-phy-test.cc > CMakeFiles/wifi-phy-test.dir/wifi-phy-test.cc.i

src/wifi/examples/CMakeFiles/wifi-phy-test.dir/wifi-phy-test.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/wifi-phy-test.dir/wifi-phy-test.cc.s"
	cd /home/espen/ns-3/ns3-wip/cmake-cache/src/wifi/examples && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -Winvalid-pch -include /home/espen/ns-3/ns3-wip/cmake-cache/CMakeFiles/stdlib_pch_exec.dir/cmake_pch.hxx -S /home/espen/ns-3/ns3-wip/src/wifi/examples/wifi-phy-test.cc -o CMakeFiles/wifi-phy-test.dir/wifi-phy-test.cc.s

# Object files for target wifi-phy-test
wifi__phy__test_OBJECTS = \
"CMakeFiles/wifi-phy-test.dir/wifi-phy-test.cc.o"

# External object files for target wifi-phy-test
wifi__phy__test_EXTERNAL_OBJECTS =

/home/espen/ns-3/ns3-wip/build/src/wifi/examples/ns3.36.1-wifi-phy-test-debug: src/wifi/examples/CMakeFiles/wifi-phy-test.dir/wifi-phy-test.cc.o
/home/espen/ns-3/ns3-wip/build/src/wifi/examples/ns3.36.1-wifi-phy-test-debug: src/wifi/examples/CMakeFiles/wifi-phy-test.dir/build.make
/home/espen/ns-3/ns3-wip/build/src/wifi/examples/ns3.36.1-wifi-phy-test-debug: /usr/lib/x86_64-linux-gnu/libsqlite3.so
/home/espen/ns-3/ns3-wip/build/src/wifi/examples/ns3.36.1-wifi-phy-test-debug: /usr/lib/x86_64-linux-gnu/librt.a
/home/espen/ns-3/ns3-wip/build/src/wifi/examples/ns3.36.1-wifi-phy-test-debug: src/wifi/examples/CMakeFiles/wifi-phy-test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/espen/ns-3/ns3-wip/cmake-cache/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable /home/espen/ns-3/ns3-wip/build/src/wifi/examples/ns3.36.1-wifi-phy-test-debug"
	cd /home/espen/ns-3/ns3-wip/cmake-cache/src/wifi/examples && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/wifi-phy-test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/wifi/examples/CMakeFiles/wifi-phy-test.dir/build: /home/espen/ns-3/ns3-wip/build/src/wifi/examples/ns3.36.1-wifi-phy-test-debug
.PHONY : src/wifi/examples/CMakeFiles/wifi-phy-test.dir/build

src/wifi/examples/CMakeFiles/wifi-phy-test.dir/clean:
	cd /home/espen/ns-3/ns3-wip/cmake-cache/src/wifi/examples && $(CMAKE_COMMAND) -P CMakeFiles/wifi-phy-test.dir/cmake_clean.cmake
.PHONY : src/wifi/examples/CMakeFiles/wifi-phy-test.dir/clean

src/wifi/examples/CMakeFiles/wifi-phy-test.dir/depend:
	cd /home/espen/ns-3/ns3-wip/cmake-cache && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/espen/ns-3/ns3-wip /home/espen/ns-3/ns3-wip/src/wifi/examples /home/espen/ns-3/ns3-wip/cmake-cache /home/espen/ns-3/ns3-wip/cmake-cache/src/wifi/examples /home/espen/ns-3/ns3-wip/cmake-cache/src/wifi/examples/CMakeFiles/wifi-phy-test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/wifi/examples/CMakeFiles/wifi-phy-test.dir/depend

