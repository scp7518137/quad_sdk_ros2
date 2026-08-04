cmake_minimum_required(VERSION 3.8)
project(gazebo_scripts)

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find dependencies
find_package(ament_cmake REQUIRED)
find_package(gazebo REQUIRED)
find_package(gazebo_ros REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(gazebo_msgs REQUIRED)
find_package(quad_msgs REQUIRED)
find_package(quad_utils REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(urdf REQUIRED)
find_package(angles REQUIRED)
find_package(controller_interface REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(realtime_tools REQUIRED)
find_package(pluginlib REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)

# Gazebo settings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GAZEBO_CXX_FLAGS}")

# --- Gazebo Sensor Plugin: contact ---
add_library(contact SHARED src/contact_plugin.cpp)
target_include_directories(contact PRIVATE include)
ament_target_dependencies(contact
  gazebo_ros rclcpp quad_msgs geometry_msgs)
target_link_libraries(contact ${GAZEBO_LIBRARIES})

# --- Gazebo Controller Plugin (ros2_control) ---
add_library(quad_controller SHARED src/controller_plugin.cpp)
target_include_directories(quad_controller PRIVATE include)
ament_target_dependencies(quad_controller
  rclcpp rclcpp_lifecycle controller_interface hardware_interface
  realtime_tools pluginlib quad_msgs quad_utils urdf angles)
target_link_libraries(quad_controller ${GAZEBO_LIBRARIES})

# --- Gazebo Model Plugin: ground truth estimator ---
add_library(ground_truth_estimator SHARED src/estimator_plugin.cpp)
target_include_directories(ground_truth_estimator PRIVATE include)
ament_target_dependencies(ground_truth_estimator
  gazebo_ros rclcpp quad_msgs quad_utils tf2 tf2_ros)
target_link_libraries(ground_truth_estimator ${GAZEBO_LIBRARIES})

# --- Standalone node: contact_state_publisher ---
add_library(contact_state_publisher SHARED src/contact_state_publisher.cpp)
target_include_directories(contact_state_publisher PRIVATE include)
ament_target_dependencies(contact_state_publisher
  rclcpp quad_msgs quad_utils gazebo_msgs tf2 tf2_ros tf2_geometry_msgs)

add_executable(contact_state_publisher_node src/contact_state_publisher_node.cpp)
target_link_libraries(contact_state_publisher_node contact_state_publisher)
target_include_directories(contact_state_publisher_node PRIVATE include)
ament_target_dependencies(contact_state_publisher_node rclcpp)

# --- Install ---
install(TARGETS
  contact quad_controller ground_truth_estimator
  contact_state_publisher contact_state_publisher_node
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/ DESTINATION include/${PROJECT_NAME})

# Plugin export for quad_controller
pluginlib_export_plugin_description_file(controller_interface
  spirit_controller_plugin.xml)

# Install config and worlds
install(DIRECTORY config worlds
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
