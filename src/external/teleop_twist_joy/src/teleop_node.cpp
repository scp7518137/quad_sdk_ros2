#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "teleop_twist_joy/teleop_twist_joy.h"

int main(int argc, char *argv[])
{
  rclcpp::init(argc, argv);
  auto node = rclcpp::Node::make_shared("teleop_twist_joy",
    rclcpp::NodeOptions()
      .allow_undeclared_parameters(true)
      .automatically_declare_parameters_from_overrides(true));

  teleop_twist_joy::TeleopTwistJoy joy_teleop(node);

  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
