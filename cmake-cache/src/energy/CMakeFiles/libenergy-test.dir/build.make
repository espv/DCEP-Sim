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
include src/energy/CMakeFiles/libenergy-test.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/energy/CMakeFiles/libenergy-test.dir/compiler_depend.make

# Include the progress variables for this target.
include src/energy/CMakeFiles/libenergy-test.dir/progress.make

# Include the compile flags for this target's objects.
include src/energy/CMakeFiles/libenergy-test.dir/flags.make

src/energy/CMakeFiles/libenergy-test.dir/test/basic-energy-harvester-test.cc.o: src/energy/CMakeFiles/libenergy-test.dir/flags.make
src/energy/CMakeFiles/libenergy-test.dir/test/basic-energy-harvester-test.cc.o: /home/espen/ns-3/ns3-wip/src/energy/test/basic-energy-harvester-test.cc
src/energy/CMakeFiles/libenergy-test.dir/test/basic-energy-harvester-test.cc.o: src/energy/CMakeFiles/libenergy-test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/espen/ns-3/ns3-wip/cmake-cache/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/energy/CMakeFiles/libenergy-test.dir/test/basic-energy-harvester-test.cc.o"
	cd /home/espen/ns-3/ns3-wip/cmake-cache/src/energy && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/energy/CMakeFiles/libenergy-test.dir/test/basic-energy-harvester-test.cc.o -MF CMakeFiles/libenergy-test.dir/test/basic-energy-harvester-test.cc.o.d -o CMakeFiles/libenergy-test.dir/test/basic-energy-harvester-test.cc.o -c /home/espen/ns-3/ns3-wip/src/energy/test/basic-energy-harvester-test.cc

src/energy/CMakeFiles/libenergy-test.dir/test/basic-energy-harvester-test.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/libenergy-test.dir/test/basic-energy-harvester-test.cc.i"
	cd /home/espen/ns-3/ns3-wip/cmake-cache/src/energy && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/espen/ns-3/ns3-wip/src/energy/test/basic-energy-harvester-test.cc > CMakeFiles/libenergy-test.dir/test/basic-energy-harvester-test.cc.i

src/energy/CMakeFiles/libenergy-test.dir/test/basic-energy-harvester-test.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/libenergy-test.dir/test/basic-energy-harvester-test.cc.s"
	cd /home/espen/ns-3/ns3-wip/cmake-cache/src/energy && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/espen/ns-3/ns3-wip/src/energy/test/basic-energy-harvester-test.cc -o CMakeFiles/libenergy-test.dir/test/basic-energy-harvester-test.cc.s

src/energy/CMakeFiles/libenergy-test.dir/test/li-ion-energy-source-test.cc.o: src/energy/CMakeFiles/libenergy-test.dir/flags.make
src/energy/CMakeFiles/libenergy-test.dir/test/li-ion-energy-source-test.cc.o: /home/espen/ns-3/ns3-wip/src/energy/test/li-ion-energy-source-test.cc
src/energy/CMakeFiles/libenergy-test.dir/test/li-ion-energy-source-test.cc.o: src/energy/CMakeFiles/libenergy-test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/espen/ns-3/ns3-wip/cmake-cache/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/energy/CMakeFiles/libenergy-test.dir/test/li-ion-energy-source-test.cc.o"
	cd /home/espen/ns-3/ns3-wip/cmake-cache/src/energy && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/energy/CMakeFiles/libenergy-test.dir/test/li-ion-energy-source-test.cc.o -MF CMakeFiles/libenergy-test.dir/test/li-ion-energy-source-test.cc.o.d -o CMakeFiles/libenergy-test.dir/test/li-ion-energy-source-test.cc.o -c /home/espen/ns-3/ns3-wip/src/energy/test/li-ion-energy-source-test.cc

src/energy/CMakeFiles/libenergy-test.dir/test/li-ion-energy-source-test.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/libenergy-test.dir/test/li-ion-energy-source-test.cc.i"
	cd /home/espen/ns-3/ns3-wip/cmake-cache/src/energy && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/espen/ns-3/ns3-wip/src/energy/test/li-ion-energy-source-test.cc > CMakeFiles/libenergy-test.dir/test/li-ion-energy-source-test.cc.i

src/energy/CMakeFiles/libenergy-test.dir/test/li-ion-energy-source-test.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/libenergy-test.dir/test/li-ion-energy-source-test.cc.s"
	cd /home/espen/ns-3/ns3-wip/cmake-cache/src/energy && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/espen/ns-3/ns3-wip/src/energy/test/li-ion-energy-source-test.cc -o CMakeFiles/libenergy-test.dir/test/li-ion-energy-source-test.cc.s

# Object files for target libenergy-test
libenergy__test_OBJECTS = \
"CMakeFiles/libenergy-test.dir/test/basic-energy-harvester-test.cc.o" \
"CMakeFiles/libenergy-test.dir/test/li-ion-energy-source-test.cc.o"

# External object files for target libenergy-test
libenergy__test_EXTERNAL_OBJECTS =

/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-energy-test-debug.so: src/energy/CMakeFiles/libenergy-test.dir/test/basic-energy-harvester-test.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-energy-test-debug.so: src/energy/CMakeFiles/libenergy-test.dir/test/li-ion-energy-source-test.cc.o
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-energy-test-debug.so: src/energy/CMakeFiles/libenergy-test.dir/build.make
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-energy-test-debug.so: /usr/lib/x86_64-linux-gnu/librt.a
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-energy-test-debug.so: /usr/lib/x86_64-linux-gnu/libsqlite3.so
/home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-energy-test-debug.so: src/energy/CMakeFiles/libenergy-test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/espen/ns-3/ns3-wip/cmake-cache/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX shared library /home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-energy-test-debug.so"
	cd /home/espen/ns-3/ns3-wip/cmake-cache/src/energy && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/libenergy-test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/energy/CMakeFiles/libenergy-test.dir/build: /home/espen/ns-3/ns3-wip/build/lib/libns3.36.1-energy-test-debug.so
.PHONY : src/energy/CMakeFiles/libenergy-test.dir/build

src/energy/CMakeFiles/libenergy-test.dir/clean:
	cd /home/espen/ns-3/ns3-wip/cmake-cache/src/energy && $(CMAKE_COMMAND) -P CMakeFiles/libenergy-test.dir/cmake_clean.cmake
.PHONY : src/energy/CMakeFiles/libenergy-test.dir/clean

src/energy/CMakeFiles/libenergy-test.dir/depend:
	cd /home/espen/ns-3/ns3-wip/cmake-cache && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/espen/ns-3/ns3-wip /home/espen/ns-3/ns3-wip/src/energy /home/espen/ns-3/ns3-wip/cmake-cache /home/espen/ns-3/ns3-wip/cmake-cache/src/energy /home/espen/ns-3/ns3-wip/cmake-cache/src/energy/CMakeFiles/libenergy-test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/energy/CMakeFiles/libenergy-test.dir/depend

