add_executable(${PROJECT_NAME}_mocap_node
  natnet/natnet_packet_definition.cpp
  natnet/natnet_messages.cpp
  socket.cpp
  version.cpp
  mocap_node.cpp
  mocap_config.cpp
  data_model.cpp
  rigid_body_publisher.cpp
)

ament_target_dependencies(${PROJECT_NAME}_mocap_node
  rclcpp
  geometry_msgs
  tf2_ros
)

set_target_properties(${PROJECT_NAME}_mocap_node PROPERTIES
                      OUTPUT_NAME mocap_node PREFIX "")

install(TARGETS
  ${PROJECT_NAME}_mocap_node
  DESTINATION lib/${PROJECT_NAME}
)
