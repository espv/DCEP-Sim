#target_link_libraries(name ${catkin_LIBRARIES}  ${YAMLCPP_LIBRARIES})

find_package(yaml-cpp REQUIRED)
include_directories(${YAML_CPP_INCLUDE_DIR})
set(yaml_cpp_libraries)
if(${yaml-cpp_FOUND})
    message(STATUS "YAML LIBRARIES: ${YAML_CPP_LIBRARIES}")
    message(STATUS "YAML library: ${YAML_CPP_LIBRARY}")
    message(STATUS "YAML library: ${yaml-cpp_LIBRARY}")
    message(STATUS "YAML library: ${yaml-cpp_LIBRARIES}")
    message(STATUS "YAML library: ${YAML_CPP_SHARED_LIBS_BUILT}")
    set(yaml_cpp_libraries
            ${YAML_CPP_LIBRARY_DIR}
    )
endif()

set(Boost_USE_STATIC_LIBS        ON)  # only find static libs
set(Boost_USE_DEBUG_LIBS        OFF)  # ignore debug libs and
set(Boost_USE_RELEASE_LIBS       ON)  # only find release libs
find_package(Boost COMPONENTS regex serialization)
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

if(${Boost_FOUND})
    message(STATUS "Boost found")
    message(STATUS "${Boost_INCLUDE_DIR}")
    message(STATUS "${Boost_LIBRARIES}")
    #message(STATUS "${Boost_LIBRARIES}")
else()
    message(STATUS "Boost not found")
endif()

build_lib(
  LIBNAME dcep
  SOURCE_FILES
    model/communication.cc
    model/placement.cc
    model/cep-engine.cc
    model/dcep.cc
    model/dcep-header.cc
    helper/dcep-app-helper.cc
    model/utility/CSVIterator.cpp
    model/utility/knapsack_solver.cpp
    model/utility/utility.cpp
    model/dcep-apps/expose-coordinator.cc
    model/dcep-apps/expose-wrapper.cc
    model/dcep-apps/dataset-manipulator.cc
  HEADER_FILES
    model/communication.h
    model/placement.h
    model/cep-engine.h
    model/dcep.h
    model/common.h
    model/message-types.h
    model/dcep-header.h
    helper/dcep-app-helper.h
    model/utility/CSVIterator.h
    model/utility/knapsack_solver.h
    model/utility/utility.h
    model/dcep-apps/expose-coordinator.h
    model/dcep-apps/expose-wrapper.h
    model/dcep-apps/dataset-manipulator.h
  LIBRARIES_TO_LINK
        ${libstream-processing}
        ${libcore}
        ${libolsr}
        ${libnetanim}
        ${YAML_CPP_LIBRARIES}
        ${Boost_LIBRARIES}
  TEST_SOURCES
        test/dcep-test-suite.cc
)
