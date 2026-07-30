#include <rclcpp/rclcpp.hpp>

#include "quad_utils/rviz_interface.h"

int main(int argc, char** argv) {
  // Announce this program to the ROS 2 network
  rclcpp::init(argc, argv);
  rclcpp::NodeOptions options;
  options.allow_undeclared_parameters(true);
  options.automatically_declare_parameters_from_overrides(true);
  auto node =
      std::make_shared<rclcpp::Node>("rviz_interface_node", options);

  RVizInterface rviz_interface(node);
  rviz_interface.spin();

  rclcpp::shutdown();
  return 0;
}
