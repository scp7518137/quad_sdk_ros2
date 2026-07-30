#include <rclcpp/rclcpp.hpp>

#include <iostream>

#include "local_planner/local_planner.h"

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);

  rclcpp::NodeOptions options;
  options.allow_undeclared_parameters(true);
  options.automatically_declare_parameters_from_overrides(true);
  auto nh = std::make_shared<rclcpp::Node>("local_planner_node", options);

  LocalPlanner local_planner(nh);
  local_planner.spin();

  rclcpp::shutdown();
  return 0;
}
