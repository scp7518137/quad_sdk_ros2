#include <rclcpp/rclcpp.hpp>

#include "global_body_planner/global_body_planner.h"

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);

  rclcpp::NodeOptions options;
  options.allow_undeclared_parameters(true);
  options.automatically_declare_parameters_from_overrides(true);
  auto node = std::make_shared<rclcpp::Node>("global_body_planner", options);

  GlobalBodyPlanner global_body_planner(node);
  global_body_planner.spin();

  rclcpp::shutdown();
  return 0;
}
