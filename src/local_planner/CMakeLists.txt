cmake_minimum_required(VERSION 3.8)
project(local_planner)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Allow undefined symbols in transitive shared libraries (e.g. libgdal.so, pulled
# in via PCL through quad_utils) to be resolved at load time by the system
# libcurl/libtiff. See quad_utils/CMakeLists.txt for details.
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--allow-shlib-undefined")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--allow-shlib-undefined")

## Set default cmake build type to release
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(quad_msgs REQUIRED)
find_package(quad_utils REQUIRED)
find_package(grid_map_core REQUIRED)
find_package(grid_map_ros REQUIRED)
find_package(grid_map_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(nmpc_controller REQUIRED)
find_package(Eigen3 REQUIRED)

# RBDL is needed transitively through quad_utils (QuadKD). Use the vendored
# FindRBDL.cmake module from quad_utils.
LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../quad_utils)
FIND_PACKAGE(RBDL COMPONENTS URDFREADER REQUIRED)

include_directories(
  include
  ${EIGEN3_INCLUDE_DIRS}
  ${RBDL_INCLUDE_DIR}
  /usr/include/coin
)
link_directories(/usr/lib)

# apt 包 coinor-libipopt-dev 的 IpoptConfig.h (3.11.9) 未定义 HAVE_CSTDDEF /
# HAVE_CASSERT 等宏，导致 Ipopt 头触发 "#error don't have header file"。
add_compile_definitions(HAVE_CSTDDEF HAVE_CASSERT)

## Declare a C++ library for control
add_library(${PROJECT_NAME}
  src/local_planner.cpp
  src/local_footstep_planner.cpp
)

ament_target_dependencies(${PROJECT_NAME}
  rclcpp
  std_msgs
  geometry_msgs
  quad_msgs
  quad_utils
  grid_map_core
  grid_map_ros
  grid_map_msgs
  tf2
  nmpc_controller
)

## Declare a C++ executable
add_executable(${PROJECT_NAME}_node src/local_planner_node.cpp)

target_link_libraries(${PROJECT_NAME}_node
  ${PROJECT_NAME}
)

ament_target_dependencies(${PROJECT_NAME}_node
  rclcpp
  quad_msgs
  quad_utils
  grid_map_ros
  grid_map_msgs
  nmpc_controller
)

# Static-lib ordering: nmpc_controller -> quad_utils -> RBDL, appended last.
target_link_libraries(${PROJECT_NAME}_node
  ${nmpc_controller_LIBRARIES}
  ${quad_utils_LIBRARIES}
  ${RBDL_LIBRARY}
  ${RBDL_URDFREADER_LIBRARY}
  ipopt
)

#############
## Testing ##
#############

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)

  ament_add_gtest(local_footstep_planner-test test/test_local_footstep_planner.cpp)
  target_link_libraries(local_footstep_planner-test ${PROJECT_NAME})
  ament_target_dependencies(local_footstep_planner-test
    rclcpp quad_msgs quad_utils grid_map_core grid_map_ros grid_map_msgs tf2 nmpc_controller)
  target_link_libraries(local_footstep_planner-test
    ${nmpc_controller_LIBRARIES}
    ${quad_utils_LIBRARIES}
    ${RBDL_LIBRARY}
    ${RBDL_URDFREADER_LIBRARY}
    ipopt
  )

  ament_add_gtest(local_planner-test test/test_local_planner.cpp)
  target_link_libraries(local_planner-test ${PROJECT_NAME})
  ament_target_dependencies(local_planner-test
    rclcpp quad_msgs quad_utils grid_map_core grid_map_ros grid_map_msgs tf2 nmpc_controller)
  target_link_libraries(local_planner-test
    ${nmpc_controller_LIBRARIES}
    ${quad_utils_LIBRARIES}
    ${RBDL_LIBRARY}
    ${RBDL_URDFREADER_LIBRARY}
    ipopt
  )
endif()

ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_export_dependencies(rclcpp std_msgs geometry_msgs quad_msgs quad_utils grid_map_core grid_map_ros grid_map_msgs tf2 nmpc_controller)

install(TARGETS
  ${PROJECT_NAME}_node
  DESTINATION lib/${PROJECT_NAME}
)

install(TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include)

# Install parameters
install(FILES local_planner.yaml
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
