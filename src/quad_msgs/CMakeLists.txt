cmake_minimum_required(VERSION 3.8)
project(quad_msgs)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
# find_package(catkin REQUIRED COMPONENTS
#   sensor_msgs
#   std_msgs
#   nav_msgs
#   message_generation
# )

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)

################################################
## Declare ROS messages, services and actions ##
################################################

## Generate messages in the 'msg' folder
# add_message_files(
#   FILES
#   RobotState.msg
#   BodyState.msg
#   GRFArray.msg
#   RobotPlan.msg
#   RobotPlanDiagnostics.msg
#   FootState.msg
#   MultiFootState.msg
#   MultiFootPlanContinuous.msg
#   FootPlanDiscrete.msg
#   MultiFootPlanDiscrete.msg
#   LegContactMode.msg
#   ContactMode.msg
#   MotorCommand.msg
#   LegCommand.msg
#   LegCommandArray.msg
# )

## Generate added messages and services with any dependencies listed here
# generate_messages(
# 	DEPENDENCIES
# 	sensor_msgs
# 	std_msgs
# 	nav_msgs
# 	geometry_msgs
# )

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/RobotState.msg"
  "msg/BodyState.msg"
  "msg/GRFArray.msg"
  "msg/RobotPlan.msg"
  "msg/RobotPlanDiagnostics.msg"
  "msg/FootState.msg"
  "msg/MultiFootState.msg"
  "msg/MultiFootPlanContinuous.msg"
  "msg/FootPlanDiscrete.msg"
  "msg/MultiFootPlanDiscrete.msg"
  "msg/LegContactMode.msg"
  "msg/ContactMode.msg"
  "msg/MotorCommand.msg"
  "msg/LegCommand.msg"
  "msg/LegCommandArray.msg"

  DEPENDENCIES
    sensor_msgs
    std_msgs
    nav_msgs
    geometry_msgs
    builtin_interfaces
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

###################################
## catkin specific configuration ##
###################################
## The catkin_package macro generates cmake config files for your package
## Declare things to be passed to dependent projects
## INCLUDE_DIRS: uncomment this if your package contains header files
## LIBRARIES: libraries you create in this project that dependent projects also need
## CATKIN_DEPENDS: catkin_packages dependent projects also need
## DEPENDS: system dependencies of this project that dependent projects also need
# catkin_package(
#    CATKIN_DEPENDS sensor_msgs std_msgs message_runtime
# )
ament_package()
