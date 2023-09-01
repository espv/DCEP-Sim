# CMake generated Testfile for 
# Source directory: /home/espen/ns-3/ns3-wip/utils
# Build directory: /home/espen/ns-3/ns3-wip/cmake-cache/utils
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[ctest-test-runner]=] "ns3.36.1-test-runner-debug")
set_tests_properties([=[ctest-test-runner]=] PROPERTIES  WORKING_DIRECTORY "/home/espen/ns-3/ns3-wip/build/utils/" _BACKTRACE_TRIPLES "/home/espen/ns-3/ns3-wip/build-support/macros-and-definitions.cmake;1344;add_test;/home/espen/ns-3/ns3-wip/utils/CMakeLists.txt;15;set_runtime_outputdirectory;/home/espen/ns-3/ns3-wip/utils/CMakeLists.txt;0;")
add_test([=[ctest-bench-simulator]=] "ns3.36.1-bench-simulator-debug")
set_tests_properties([=[ctest-bench-simulator]=] PROPERTIES  WORKING_DIRECTORY "/home/espen/ns-3/ns3-wip/build/utils/" _BACKTRACE_TRIPLES "/home/espen/ns-3/ns3-wip/build-support/macros-and-definitions.cmake;1344;add_test;/home/espen/ns-3/ns3-wip/utils/CMakeLists.txt;23;set_runtime_outputdirectory;/home/espen/ns-3/ns3-wip/utils/CMakeLists.txt;0;")
add_test([=[ctest-bench-packets]=] "ns3.36.1-bench-packets-debug")
set_tests_properties([=[ctest-bench-packets]=] PROPERTIES  WORKING_DIRECTORY "/home/espen/ns-3/ns3-wip/build/utils/" _BACKTRACE_TRIPLES "/home/espen/ns-3/ns3-wip/build-support/macros-and-definitions.cmake;1344;add_test;/home/espen/ns-3/ns3-wip/utils/CMakeLists.txt;30;set_runtime_outputdirectory;/home/espen/ns-3/ns3-wip/utils/CMakeLists.txt;0;")
add_test([=[ctest-print-introspected-doxygen]=] "ns3.36.1-print-introspected-doxygen-debug")
set_tests_properties([=[ctest-print-introspected-doxygen]=] PROPERTIES  WORKING_DIRECTORY "/home/espen/ns-3/ns3-wip/build/utils/" _BACKTRACE_TRIPLES "/home/espen/ns-3/ns3-wip/build-support/macros-and-definitions.cmake;1344;add_test;/home/espen/ns-3/ns3-wip/utils/CMakeLists.txt;40;set_runtime_outputdirectory;/home/espen/ns-3/ns3-wip/utils/CMakeLists.txt;0;")
add_test([=[ctest-perf-io]=] "ns3.36.1-perf-io-debug")
set_tests_properties([=[ctest-perf-io]=] PROPERTIES  WORKING_DIRECTORY "/home/espen/ns-3/ns3-wip/build/utils/perf/" _BACKTRACE_TRIPLES "/home/espen/ns-3/ns3-wip/build-support/macros-and-definitions.cmake;1344;add_test;/home/espen/ns-3/ns3-wip/utils/CMakeLists.txt;48;set_runtime_outputdirectory;/home/espen/ns-3/ns3-wip/utils/CMakeLists.txt;0;")
