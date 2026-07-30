#include <rclcpp/rclcpp.hpp>

#include "robot_driver/robot_driver.h"

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);

  rclcpp::NodeOptions options;
  options.allow_undeclared_parameters(true);
  options.automatically_declare_parameters_from_overrides(true);
  auto node = std::make_shared<rclcpp::Node>("robot_driver", options);

  RobotDriver robot_driver(node, argc, argv);
  robot_driver.spin();

  rclcpp::shutdown();
  return 0;
}
