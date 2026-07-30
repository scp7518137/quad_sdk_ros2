#include <rclcpp/rclcpp.hpp>

#include "quad_utils/trajectory_publisher.h"

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  rclcpp::NodeOptions options;
  options.allow_undeclared_parameters(true);
  options.automatically_declare_parameters_from_overrides(true);
  auto node = std::make_shared<rclcpp::Node>("trajectory_publisher_node", options);

  TrajectoryPublisher trajectory_publisher(node);
  trajectory_publisher.spin();

  rclcpp::shutdown();
  return 0;
}
