cmake_minimum_required(VERSION 3.5)
project(nmpc_controller)

# Allow shlib-undefined for PCL->GDAL transitive libs (curl/tiff versioned symbols)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--allow-shlib-undefined")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--allow-shlib-undefined")

# Compile as C++17 (matches quad_utils/robot_driver)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-ffast-math)

# Set default cmake build type to release
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(quad_msgs REQUIRED)
find_package(quad_utils REQUIRED)
find_package(grid_map_core REQUIRED)
find_package(grid_map_ros REQUIRED)
find_package(Eigen3 REQUIRED)

# RBDL (QuadKD in quad_utils depends on it; quad_utils is a static lib in the
# install space, so every downstream consumer must link RBDL itself, ordered
# after quad_utils. FindRBDL.cmake lives in the quad_utils package source dir.
LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../quad_utils)
FIND_PACKAGE(RBDL COMPONENTS URDFREADER REQUIRED)

include_directories(
  include
  ${EIGEN3_INCLUDE_DIRS}
  /usr/include/coin
)
link_directories(/usr/lib)

# apt 包 coinor-libipopt-dev 的 IpoptConfig.h (3.11.9) 未定义 HAVE_CSTDDEF /
# HAVE_CASSERT 等宏，导致 IpSmartPtr.hpp / IpDebug.hpp 触发
# "#error don't have header file"。这些对应 <cstddef> / <cassert>，标准 C++ 头，
# 直接在此定义以满足 Ipopt 头文件的可用性检查。
add_compile_definitions(HAVE_CSTDDEF HAVE_CASSERT)

# Declare a C++ library for control
add_library(nmpc_controller
  src/gen/eval_g_spirit.cpp
  src/gen/eval_jac_g_spirit.cpp
  src/gen/eval_hess_g_spirit.cpp
  src/gen/eval_g_a1.cpp
  src/gen/eval_jac_g_a1.cpp
  src/gen/eval_hess_g_a1.cpp
  src/gen/eval_g_leg_simple.cpp
  src/gen/eval_jac_g_leg_simple.cpp
  src/gen/eval_hess_g_leg_simple.cpp
  src/gen/eval_g_leg_complex.cpp
  src/gen/eval_jac_g_leg_complex.cpp
  src/gen/eval_hess_g_leg_complex.cpp
  src/gen/eval_g_leg_simple_to_complex.cpp
  src/gen/eval_jac_g_leg_simple_to_complex.cpp
  src/gen/eval_hess_g_leg_simple_to_complex.cpp
  src/gen/eval_g_leg_complex_to_simple.cpp
  src/gen/eval_jac_g_leg_complex_to_simple.cpp
  src/gen/eval_hess_g_leg_complex_to_simple.cpp
  src/quad_nlp_utils.cpp
  src/quad_nlp.cpp
  src/adaptive_complexity_utils.cpp
  src/nmpc_controller.cpp
)
ament_target_dependencies(nmpc_controller
  rclcpp
  quad_msgs
  quad_utils
  grid_map_core
  grid_map_ros
)
target_link_libraries(nmpc_controller
  Eigen3::Eigen
  ipopt
  ${RBDL_LIBRARY}
  ${RBDL_URDFREADER_LIBRARY}
)

ament_export_include_directories(include)
ament_export_libraries(nmpc_controller)
ament_export_dependencies(rclcpp quad_msgs quad_utils grid_map_core grid_map_ros)

install(DIRECTORY include/ DESTINATION include)

install(TARGETS nmpc_controller
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(nmpc_controller-test
    test/test_nmpc_controller.cpp
    test/test_adaptive_complexity.cpp)
  target_link_libraries(nmpc_controller-test nmpc_controller
    ${RBDL_LIBRARY}
    ${RBDL_URDFREADER_LIBRARY})
  ament_target_dependencies(nmpc_controller-test
    rclcpp
    quad_msgs
    quad_utils)
endif()

# Install parameters
install(FILES nmpc_controller.yaml
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
