cmake_minimum_required(VERSION 3.8)
project(quad_utils)

## Compile as C++17 (required by ROS 2 Humble's rclcpp)
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Allow undefined symbols in transitive shared libraries (e.g. libgdal.so, pulled
# in via PCL) to be resolved at load time by the system libcurl/libtiff. Without
# this, the linker errors on versioned symbols (curl_easy_init@CURL_OPENSSL_4,
# TIFFReadFromUserBuffer@LIBTIFF_4.0) it cannot see on the command line but which
# are present at runtime.
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--allow-shlib-undefined")

## Set default cmake build type to release
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()


find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(quad_msgs REQUIRED)
find_package(grid_map_core REQUIRED)
find_package(grid_map_ros REQUIRED)
find_package(grid_map_pcl REQUIRED)
find_package(grid_map_msgs REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(pcl_msgs REQUIRED)
find_package(visualization_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(pcl_conversions REQUIRED)

find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
find_package(Eigen3 REQUIRED)

# We need to add the project source path to the CMake module path so that
# the FindRBDL.cmake script can be found.
LIST( APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR} )

# Search for the RBDL include directory and library
FIND_PACKAGE (RBDL COMPONENTS URDFREADER REQUIRED)


###########
## Build ##
###########

## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(
  include
  ${EIGEN3_INCLUDE_DIRS}
  ${Python3_INCLUDE_DIRS}
)

# Add the include directory to the include paths
INCLUDE_DIRECTORIES ( ${RBDL_INCLUDE_DIR} ${EIGEN3_INCLUDE_DIR})


add_library(quad_utils
  src/rviz_interface.cpp
  src/fast_terrain_map.cpp
  src/terrain_map_publisher.cpp
  src/trajectory_publisher.cpp
  src/mesh_to_grid_map_converter.cpp
  src/math_utils.cpp
  src/ros_utils.cpp
  src/quad_kd.cpp
  src/remote_heartbeat.cpp
)

# Link RBDL into quad_utils (plain signature, compatible with
# ament_target_dependencies below). RBDL is a static library whose symbols
# are needed by any downstream consumer of QuadKD, so each downstream package
# links RBDL itself (ordered after quad_utils via a link group).
target_link_libraries(quad_utils ${RBDL_LIBRARY} ${RBDL_URDFREADER_LIBRARY})

ament_target_dependencies(quad_utils
  rclcpp
  std_msgs
  nav_msgs
  geometry_msgs
  sensor_msgs
  quad_msgs
  grid_map_core
  grid_map_ros
  grid_map_msgs
  grid_map_pcl
  ament_index_cpp
  pcl_msgs
  pcl_conversions
  visualization_msgs
  tf2_ros
  tf2_geometry_msgs
)

## Declare a C++ executable

add_executable(rviz_interface_node src/rviz_interface_node.cpp)
add_executable(terrain_map_publisher_node src/terrain_map_publisher_node.cpp)
add_executable(trajectory_publisher_node src/trajectory_publisher_node.cpp)
add_executable(mesh_to_grid_map_node src/mesh_to_grid_map_node.cpp)
add_executable(remote_heartbeat_node src/remote_heartbeat_node.cpp)

target_link_libraries(rviz_interface_node quad_utils)
ament_target_dependencies(rviz_interface_node rclcpp quad_msgs visualization_msgs tf2_ros tf2_geometry_msgs geometry_msgs nav_msgs sensor_msgs)

target_link_libraries(terrain_map_publisher_node quad_utils)
ament_target_dependencies(terrain_map_publisher_node rclcpp quad_msgs grid_map_msgs grid_map_ros sensor_msgs ament_index_cpp)

target_link_libraries(trajectory_publisher_node quad_utils)
ament_target_dependencies(trajectory_publisher_node rclcpp quad_msgs)

target_link_libraries(mesh_to_grid_map_node quad_utils)
ament_target_dependencies(mesh_to_grid_map_node rclcpp quad_msgs)

target_link_libraries(remote_heartbeat_node quad_utils)
ament_target_dependencies(remote_heartbeat_node rclcpp quad_msgs)


#############
## Testing ##
#############

## Add gtest based cpp test target and link libraries

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()

  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(quad_utils-test
    test/test_rviz_interface.cpp
    test/test_terrain_map_publisher.cpp
    test/test_fast_terrain_map.cpp
    test/test_quad_kd.cpp
    test/test_eigen.cpp
    test/test_math_utils.cpp
    test/test_main.cpp
  )
  target_link_libraries(quad_utils-test quad_utils)
  ament_target_dependencies(quad_utils-test rclcpp quad_msgs)
  if(Python3_FOUND)
    target_link_libraries(quad_utils-test ${Python3_LIBRARIES})
  endif()
endif()

###########
## Export ##
###########

ament_export_include_directories(include)
ament_export_libraries(quad_utils)
ament_export_dependencies(rclcpp std_msgs nav_msgs sensor_msgs geometry_msgs quad_msgs grid_map_core grid_map_ros grid_map_pcl)

install(TARGETS
  rviz_interface_node
  terrain_map_publisher_node
  trajectory_publisher_node
  mesh_to_grid_map_node
  remote_heartbeat_node
  DESTINATION lib/${PROJECT_NAME}
)

install(TARGETS quad_utils
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

# Install public headers so downstream packages can #include <quad_utils/...>
install(DIRECTORY include/ DESTINATION include)

# Install launch files, configs and resources
install(DIRECTORY launch config rviz data
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
