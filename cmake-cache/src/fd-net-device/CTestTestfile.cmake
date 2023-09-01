# CMake generated Testfile for 
# Source directory: /home/espen/ns-3/ns3-wip/src/fd-net-device
# Build directory: /home/espen/ns-3/ns3-wip/cmake-cache/src/fd-net-device
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[ctest-raw-sock-creator]=] "ns3.36.1-raw-sock-creator-debug")
set_tests_properties([=[ctest-raw-sock-creator]=] PROPERTIES  WORKING_DIRECTORY "/home/espen/ns-3/ns3-wip/build/src/fd-net-device/" _BACKTRACE_TRIPLES "/home/espen/ns-3/ns3-wip/build-support/macros-and-definitions.cmake;1344;add_test;/home/espen/ns-3/ns3-wip/src/fd-net-device/CMakeLists.txt;164;set_runtime_outputdirectory;/home/espen/ns-3/ns3-wip/src/fd-net-device/CMakeLists.txt;0;")
add_test([=[ctest-tap-device-creator]=] "ns3.36.1-tap-device-creator-debug")
set_tests_properties([=[ctest-tap-device-creator]=] PROPERTIES  WORKING_DIRECTORY "/home/espen/ns-3/ns3-wip/build/src/fd-net-device/" _BACKTRACE_TRIPLES "/home/espen/ns-3/ns3-wip/build-support/macros-and-definitions.cmake;1344;add_test;/home/espen/ns-3/ns3-wip/src/fd-net-device/CMakeLists.txt;199;set_runtime_outputdirectory;/home/espen/ns-3/ns3-wip/src/fd-net-device/CMakeLists.txt;0;")
subdirs("examples")
