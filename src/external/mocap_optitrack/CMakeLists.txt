cmake_minimum_required(VERSION 3.8)

project(mocap_optitrack)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(tf2_ros REQUIRED)

include_directories(include)

add_subdirectory(src)

install(DIRECTORY launch config
  DESTINATION share/${PROJECT_NAME})

ament_package()
