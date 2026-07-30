cmake_minimum_required(VERSION 3.8)
project(global_body_planner)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Allow undefined symbols in transitive shared libraries (e.g. libgdal.so, pulled
# in via PCL through quad_utils) to be resolved at load time by the system
# libcurl/libtiff. See quad_utils/CMakeLists.txt for details.
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--allow-shlib-undefined")

## Compile options
add_compile_options(-ffast-math)

## Set default cmake build type to release
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(quad_msgs REQUIRED)
find_package(quad_utils REQUIRED)
find_package(grid_map_core REQUIRED)
find_package(grid_map_ros REQUIRED)
find_package(grid_map_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(visualization_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(Eigen3 REQUIRED)

# Add this package's source path so the vendored FindRBDL.cmake module
# (copied from quad_utils) can be located.
LIST( APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR} )
FIND_PACKAGE (RBDL COMPONENTS URDFREADER REQUIRED)

include_directories(
  include
  ${EIGEN3_INCLUDE_DIRS}
  ${RBDL_INCLUDE_DIR}
)

set(MY_LIB_SOURCES
  src/global_body_planner.cpp
  src/planning_utils.cpp
  src/global_body_plan.cpp
  src/graph_class.cpp
  src/planner_class.cpp
  src/rrt.cpp
  src/gbpl.cpp)

# Declare a C++ library
add_library(${PROJECT_NAME}
  ${MY_LIB_SOURCES}
)

ament_target_dependencies(${PROJECT_NAME}
  rclcpp
  std_msgs
  quad_msgs
  quad_utils
  grid_map_core
  grid_map_ros
  grid_map_msgs
  nav_msgs
  geometry_msgs
  visualization_msgs
  tf2
  tf2_geometry_msgs
  tf2_ros
)

# Declare a C++ executable
add_executable(${PROJECT_NAME}_node src/global_body_planner_node.cpp)

target_link_libraries(${PROJECT_NAME}_node
  ${PROJECT_NAME}
)

ament_target_dependencies(${PROJECT_NAME}_node
  rclcpp
  quad_msgs
  quad_utils
  grid_map_ros
  nav_msgs
  geometry_msgs
  visualization_msgs
  tf2_ros
  tf2_geometry_msgs
)

# RBDL is a static library pulled in transitively through quad_utils (QuadKD).
# It must be linked *after* quad_utils, and quad_utils must be linked *after*
# the PCL/ROS deps above, so we append these as a separate, later call.
target_link_libraries(${PROJECT_NAME}_node
  ${quad_utils_LIBRARIES}
  ${RBDL_LIBRARY}
  ${RBDL_URDFREADER_LIBRARY}
)

#############
## Testing ##
#############

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()

  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(${PROJECT_NAME}-test
    test/test_global_body_planner.cpp
    test/test_kinematics.cpp
    test/test_performance.cpp
    test/test_rotate_grf.cpp)
  target_link_libraries(${PROJECT_NAME}-test ${PROJECT_NAME})
  ament_target_dependencies(${PROJECT_NAME}-test
    rclcpp
    quad_msgs
    quad_utils
    grid_map_core
    grid_map_ros
    grid_map_msgs
    nav_msgs
    geometry_msgs
    visualization_msgs
    tf2
    tf2_geometry_msgs
    tf2_ros)

  # Link RBDL after quad_utils (static-lib ordering, see node target above).
  target_link_libraries(${PROJECT_NAME}-test
    ${quad_utils_LIBRARIES}
    ${RBDL_LIBRARY}
    ${RBDL_URDFREADER_LIBRARY}
  )
endif()

ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_export_dependencies(rclcpp std_msgs quad_msgs quad_utils grid_map_core grid_map_ros)

install(TARGETS
  ${PROJECT_NAME}_node
  DESTINATION lib/${PROJECT_NAME}
)

install(TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

# Install launch files and parameters
install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)
install(FILES global_body_planner.yaml
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
