# CMake generated Testfile for 
# Source directory: /home/espen/ns-3/ns3-wip
# Build directory: /home/espen/ns-3/ns3-wip/cmake-cache
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[ctest-stdlib_pch_exec]=] "ns3.36.1-stdlib_pch_exec-debug")
set_tests_properties([=[ctest-stdlib_pch_exec]=] PROPERTIES  WORKING_DIRECTORY "/home/espen/ns-3/ns3-wip/cmake-cache/" _BACKTRACE_TRIPLES "/home/espen/ns-3/ns3-wip/build-support/macros-and-definitions.cmake;1344;add_test;/home/espen/ns-3/ns3-wip/build-support/macros-and-definitions.cmake;1275;set_runtime_outputdirectory;/home/espen/ns-3/ns3-wip/CMakeLists.txt;138;process_options;/home/espen/ns-3/ns3-wip/CMakeLists.txt;0;")
subdirs("src")
subdirs("examples")
subdirs("utils")
