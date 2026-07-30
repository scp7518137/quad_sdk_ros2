cmake_minimum_required(VERSION 3.5)
project(robot_driver)

## Set default cmake build type to release
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

## Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

## Setup MBLINK
set(MBLINK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../external/mblink/)

message(STATUS "Using architecture ${CMAKE_SYSTEM_PROCESSOR}")

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "arm" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
  set(GRMBLINK ${MBLINK_DIR}/lib/libgrmblink_tx2.a)
else()
  set(GRMBLINK ${MBLINK_DIR}/lib/libgrmblink_linux.a)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(quad_msgs REQUIRED)
find_package(quad_utils REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_eigen REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(Threads REQUIRED)

# Add quad_utils source path so its vendored FindRBDL.cmake module can be located.
LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../quad_utils)
FIND_PACKAGE(RBDL COMPONENTS URDFREADER REQUIRED)

include_directories(
  include
  ${MBLINK_DIR}/include
  ${EIGEN3_INCLUDE_DIRS}
  ${RBDL_INCLUDE_DIR}
)

# Vendored mavlink (external/mblink) headers trigger -Waddress-of-packed-member;
# suppress it for this package's targets only.
add_compile_options(-Wno-address-of-packed-member)

## Declare a C++ library for control
add_library(robot_driver
  src/robot_driver.cpp
  src/robot_driver_utils.cpp
  src/controllers/leg_controller.cpp
  src/controllers/inverse_dynamics_controller.cpp
  src/controllers/grf_pid_controller.cpp
  src/controllers/joint_controller.cpp
  src/estimators/state_estimator.cpp
  src/estimators/comp_filter_estimator.cpp
  src/estimators/ekf_estimator.cpp
  src/hardware_interfaces/hardware_interface.cpp
  src/hardware_interfaces/spirit_interface.cpp
)

ament_target_dependencies(robot_driver
  rclcpp
  quad_msgs
  quad_utils
  tf2
  tf2_eigen
  tf2_geometry_msgs
  sensor_msgs
  geometry_msgs
  std_msgs
)

target_link_libraries(robot_driver
  ${GRMBLINK}
  ${quad_utils_LIBRARIES}
  ${RBDL_LIBRARY}
  ${RBDL_URDFREADER_LIBRARY}
  Threads::Threads
)

## Declare a C++ executable
add_executable(robot_driver_node src/robot_driver_node.cpp)

ament_target_dependencies(robot_driver_node
  rclcpp
  quad_msgs
  quad_utils
)

target_link_libraries(robot_driver_node
  robot_driver
  ${quad_utils_LIBRARIES}
  ${RBDL_LIBRARY}
  ${RBDL_URDFREADER_LIBRARY}
)

install(TARGETS robot_driver robot_driver_node
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto QUIET)
  if(ament_lint_auto_FOUND)
    ament_lint_auto_find_test_dependencies()
  endif()
  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(robot_driver_test test/test_robot_driver.cpp)
  if(TARGET robot_driver_test)
    target_link_libraries(robot_driver_test robot_driver ${quad_utils_LIBRARIES} ${RBDL_LIBRARY} ${RBDL_URDFREADER_LIBRARY} Threads::Threads)
  endif()
endif()

# Install parameters
install(FILES robot_driver.yaml
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
