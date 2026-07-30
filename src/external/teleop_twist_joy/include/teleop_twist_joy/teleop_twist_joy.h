#ifndef TELEOP_TWIST_JOY_TELEOP_TWIST_JOY_H
#define TELEOP_TWIST_JOY_TELEOP_TWIST_JOY_H

#include <memory>
#include <rclcpp/rclcpp.hpp>

namespace teleop_twist_joy
{

/**
 * Class implementing a basic Joy -> Twist translation.
 */
class TeleopTwistJoy
{
public:
  explicit TeleopTwistJoy(rclcpp::Node::SharedPtr node);

private:
  struct Impl;
  Impl* pimpl_;
};

}  // namespace teleop_twist_joy

#endif  // TELEOP_TWIST_JOY_TELEOP_TWIST_JOY_H
