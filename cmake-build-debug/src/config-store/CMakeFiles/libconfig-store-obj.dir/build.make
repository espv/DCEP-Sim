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
include src/config-store/CMakeFiles/libconfig-store-obj.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/config-store/CMakeFiles/libconfig-store-obj.dir/compiler_depend.make

# Include the progress variables for this target.
include src/config-store/CMakeFiles/libconfig-store-obj.dir/progress.make

# Include the compile flags for this target's objects.
include src/config-store/CMakeFiles/libconfig-store-obj.dir/flags.make

src/config-store/CMakeFiles/libconfig-store-obj.dir/model/attribute-default-iterator.cc.o: src/config-store/CMakeFiles/libconfig-store-obj.dir/flags.make
src/config-store/CMakeFiles/libconfig-store-obj.dir/model/attribute-default-iterator.cc.o: /home/espen/ns-3/ns3-wip/src/config-store/model/attribute-default-iterator.cc
src/config-store/CMakeFiles/libconfig-store-obj.dir/model/attribute-default-iterator.cc.o: src/config-store/CMakeFiles/libconfig-store-obj.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/espen/ns-3/ns3-wip/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/config-store/CMakeFiles/libconfig-store-obj.dir/model/attribute-default-iterator.cc.o"
	cd /home/espen/ns-3/ns3-wip/cmake-build-debug/src/config-store && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/config-store/CMakeFiles/libconfig-store-obj.dir/model/attribute-default-iterator.cc.o -MF CMakeFiles/libconfig-store-obj.dir/model/attribute-default-iterator.cc.o.d -o CMakeFiles/libconfig-store-obj.dir/model/attribute-default-iterator.cc.o -c /home/espen/ns-3/ns3-wip/src/config-store/model/attribute-default-iterator.cc

src/config-store/CMakeFiles/libconfig-store-obj.dir/model/attribute-default-iterator.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/libconfig-store-obj.dir/model/attribute-default-iterator.cc.i"
	cd /home/espen/ns-3/ns3-wip/cmake-build-debug/src/config-store && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/espen/ns-3/ns3-wip/src/config-store/model/attribute-default-iterator.cc > CMakeFiles/libconfig-store-obj.dir/model/attribute-default-iterator.cc.i

src/config-store/CMakeFiles/libconfig-store-obj.dir/model/attribute-default-iterator.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/libconfig-store-obj.dir/model/attribute-default-iterator.cc.s"
	cd /home/espen/ns-3/ns3-wip/cmake-build-debug/src/config-store && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/espen/ns-3/ns3-wip/src/config-store/model/attribute-default-iterator.cc -o CMakeFiles/libconfig-store-obj.dir/model/attribute-default-iterator.cc.s

src/config-store/CMakeFiles/libconfig-store-obj.dir/model/attribute-iterator.cc.o: src/config-store/CMakeFiles/libconfig-store-obj.dir/flags.make
src/config-store/CMakeFiles/libconfig-store-obj.dir/model/attribute-iterator.cc.o: /home/espen/ns-3/ns3-wip/src/config-store/model/attribute-iterator.cc
src/config-store/CMakeFiles/libconfig-store-obj.dir/model/attribute-iterator.cc.o: src/config-store/CMakeFiles/libconfig-store-obj.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/espen/ns-3/ns3-wip/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/config-store/CMakeFiles/libconfig-store-obj.dir/model/attribute-iterator.cc.o"
	cd /home/espen/ns-3/ns3-wip/cmake-build-debug/src/config-store && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/config-store/CMakeFiles/libconfig-store-obj.dir/model/attribute-iterator.cc.o -MF CMakeFiles/libconfig-store-obj.dir/model/attribute-iterator.cc.o.d -o CMakeFiles/libconfig-store-obj.dir/model/attribute-iterator.cc.o -c /home/espen/ns-3/ns3-wip/src/config-store/model/attribute-iterator.cc

src/config-store/CMakeFiles/libconfig-store-obj.dir/model/attribute-iterator.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/libconfig-store-obj.dir/model/attribute-iterator.cc.i"
	cd /home/espen/ns-3/ns3-wip/cmake-build-debug/src/config-store && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/espen/ns-3/ns3-wip/src/config-store/model/attribute-iterator.cc > CMakeFiles/libconfig-store-obj.dir/model/attribute-iterator.cc.i

src/config-store/CMakeFiles/libconfig-store-obj.dir/model/attribute-iterator.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/libconfig-store-obj.dir/model/attribute-iterator.cc.s"
	cd /home/espen/ns-3/ns3-wip/cmake-build-debug/src/config-store && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/espen/ns-3/ns3-wip/src/config-store/model/attribute-iterator.cc -o CMakeFiles/libconfig-store-obj.dir/model/attribute-iterator.cc.s

src/config-store/CMakeFiles/libconfig-store-obj.dir/model/config-store.cc.o: src/config-store/CMakeFiles/libconfig-store-obj.dir/flags.make
src/config-store/CMakeFiles/libconfig-store-obj.dir/model/config-store.cc.o: /home/espen/ns-3/ns3-wip/src/config-store/model/config-store.cc
src/config-store/CMakeFiles/libconfig-store-obj.dir/model/config-store.cc.o: src/config-store/CMakeFiles/libconfig-store-obj.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/espen/ns-3/ns3-wip/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object src/config-store/CMakeFiles/libconfig-store-obj.dir/model/config-store.cc.o"
	cd /home/espen/ns-3/ns3-wip/cmake-build-debug/src/config-store && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/config-store/CMakeFiles/libconfig-store-obj.dir/model/config-store.cc.o -MF CMakeFiles/libconfig-store-obj.dir/model/config-store.cc.o.d -o CMakeFiles/libconfig-store-obj.dir/model/config-store.cc.o -c /home/espen/ns-3/ns3-wip/src/config-store/model/config-store.cc

src/config-store/CMakeFiles/libconfig-store-obj.dir/model/config-store.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/libconfig-store-obj.dir/model/config-store.cc.i"
	cd /home/espen/ns-3/ns3-wip/cmake-build-debug/src/config-store && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/espen/ns-3/ns3-wip/src/config-store/model/config-store.cc > CMakeFiles/libconfig-store-obj.dir/model/config-store.cc.i

src/config-store/CMakeFiles/libconfig-store-obj.dir/model/config-store.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/libconfig-store-obj.dir/model/config-store.cc.s"
	cd /home/espen/ns-3/ns3-wip/cmake-build-debug/src/config-store && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/espen/ns-3/ns3-wip/src/config-store/model/config-store.cc -o CMakeFiles/libconfig-store-obj.dir/model/config-store.cc.s

src/config-store/CMakeFiles/libconfig-store-obj.dir/model/file-config.cc.o: src/config-store/CMakeFiles/libconfig-store-obj.dir/flags.make
src/config-store/CMakeFiles/libconfig-store-obj.dir/model/file-config.cc.o: /home/espen/ns-3/ns3-wip/src/config-store/model/file-config.cc
src/config-store/CMakeFiles/libconfig-store-obj.dir/model/file-config.cc.o: src/config-store/CMakeFiles/libconfig-store-obj.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/espen/ns-3/ns3-wip/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object src/config-store/CMakeFiles/libconfig-store-obj.dir/model/file-config.cc.o"
	cd /home/espen/ns-3/ns3-wip/cmake-build-debug/src/config-store && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/config-store/CMakeFiles/libconfig-store-obj.dir/model/file-config.cc.o -MF CMakeFiles/libconfig-store-obj.dir/model/file-config.cc.o.d -o CMakeFiles/libconfig-store-obj.dir/model/file-config.cc.o -c /home/espen/ns-3/ns3-wip/src/config-store/model/file-config.cc

src/config-store/CMakeFiles/libconfig-store-obj.dir/model/file-config.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/libconfig-store-obj.dir/model/file-config.cc.i"
	cd /home/espen/ns-3/ns3-wip/cmake-build-debug/src/config-store && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/espen/ns-3/ns3-wip/src/config-store/model/file-config.cc > CMakeFiles/libconfig-store-obj.dir/model/file-config.cc.i

src/config-store/CMakeFiles/libconfig-store-obj.dir/model/file-config.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/libconfig-store-obj.dir/model/file-config.cc.s"
	cd /home/espen/ns-3/ns3-wip/cmake-build-debug/src/config-store && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/espen/ns-3/ns3-wip/src/config-store/model/file-config.cc -o CMakeFiles/libconfig-store-obj.dir/model/file-config.cc.s

src/config-store/CMakeFiles/libconfig-store-obj.dir/model/raw-text-config.cc.o: src/config-store/CMakeFiles/libconfig-store-obj.dir/flags.make
src/config-store/CMakeFiles/libconfig-store-obj.dir/model/raw-text-config.cc.o: /home/espen/ns-3/ns3-wip/src/config-store/model/raw-text-config.cc
src/config-store/CMakeFiles/libconfig-store-obj.dir/model/raw-text-config.cc.o: src/config-store/CMakeFiles/libconfig-store-obj.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/espen/ns-3/ns3-wip/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object src/config-store/CMakeFiles/libconfig-store-obj.dir/model/raw-text-config.cc.o"
	cd /home/espen/ns-3/ns3-wip/cmake-build-debug/src/config-store && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/config-store/CMakeFiles/libconfig-store-obj.dir/model/raw-text-config.cc.o -MF CMakeFiles/libconfig-store-obj.dir/model/raw-text-config.cc.o.d -o CMakeFiles/libconfig-store-obj.dir/model/raw-text-config.cc.o -c /home/espen/ns-3/ns3-wip/src/config-store/model/raw-text-config.cc

src/config-store/CMakeFiles/libconfig-store-obj.dir/model/raw-text-config.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/libconfig-store-obj.dir/model/raw-text-config.cc.i"
	cd /home/espen/ns-3/ns3-wip/cmake-build-debug/src/config-store && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/espen/ns-3/ns3-wip/src/config-store/model/raw-text-config.cc > CMakeFiles/libconfig-store-obj.dir/model/raw-text-config.cc.i

src/config-store/CMakeFiles/libconfig-store-obj.dir/model/raw-text-config.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/libconfig-store-obj.dir/model/raw-text-config.cc.s"
	cd /home/espen/ns-3/ns3-wip/cmake-build-debug/src/config-store && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/espen/ns-3/ns3-wip/src/config-store/model/raw-text-config.cc -o CMakeFiles/libconfig-store-obj.dir/model/raw-text-config.cc.s

libconfig-store-obj: src/config-store/CMakeFiles/libconfig-store-obj.dir/model/attribute-default-iterator.cc.o
libconfig-store-obj: src/config-store/CMakeFiles/libconfig-store-obj.dir/model/attribute-iterator.cc.o
libconfig-store-obj: src/config-store/CMakeFiles/libconfig-store-obj.dir/model/config-store.cc.o
libconfig-store-obj: src/config-store/CMakeFiles/libconfig-store-obj.dir/model/file-config.cc.o
libconfig-store-obj: src/config-store/CMakeFiles/libconfig-store-obj.dir/model/raw-text-config.cc.o
libconfig-store-obj: src/config-store/CMakeFiles/libconfig-store-obj.dir/build.make
.PHONY : libconfig-store-obj

# Rule to build all files generated by this target.
src/config-store/CMakeFiles/libconfig-store-obj.dir/build: libconfig-store-obj
.PHONY : src/config-store/CMakeFiles/libconfig-store-obj.dir/build

src/config-store/CMakeFiles/libconfig-store-obj.dir/clean:
	cd /home/espen/ns-3/ns3-wip/cmake-build-debug/src/config-store && $(CMAKE_COMMAND) -P CMakeFiles/libconfig-store-obj.dir/cmake_clean.cmake
.PHONY : src/config-store/CMakeFiles/libconfig-store-obj.dir/clean

src/config-store/CMakeFiles/libconfig-store-obj.dir/depend:
	cd /home/espen/ns-3/ns3-wip/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/espen/ns-3/ns3-wip /home/espen/ns-3/ns3-wip/src/config-store /home/espen/ns-3/ns3-wip/cmake-build-debug /home/espen/ns-3/ns3-wip/cmake-build-debug/src/config-store /home/espen/ns-3/ns3-wip/cmake-build-debug/src/config-store/CMakeFiles/libconfig-store-obj.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/config-store/CMakeFiles/libconfig-store-obj.dir/depend
