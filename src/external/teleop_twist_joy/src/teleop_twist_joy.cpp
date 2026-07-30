#include "teleop_twist_joy/teleop_twist_joy.h"

#include <map>
#include <string>
#include <functional>

#include "geometry_msgs/msg/twist.hpp"
#include "sensor_msgs/msg/joy.hpp"
#include "std_msgs/msg/u_int8.hpp"
#include "rclcpp/rclcpp.hpp"

namespace teleop_twist_joy {

static const rclcpp::Logger LOGGER = rclcpp::get_logger("TeleopTwistJoy");

/**
 * Internal members of class. This is the pimpl idiom, and allows more
 * flexibility in adding parameters later without breaking ABI compatibility,
 * for robots which link TeleopTwistJoy directly into base nodes.
 */
struct TeleopTwistJoy::Impl {
  void joyCallback(const sensor_msgs::msg::Joy::ConstSharedPtr& joy);
  void sendCmdVelMsg(const sensor_msgs::msg::Joy::ConstSharedPtr& joy_msg,
                     const std::string& which_map);

  rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr joy_sub;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_pub;
  rclcpp::Publisher<std_msgs::msg::UInt8>::SharedPtr control_mode_pub;

  int enable_button;
  int enable_turbo_button;
  int stand_button;
  int sit_button;
  int safety_button;

  std::map<std::string, int> axis_linear_map;
  std::map<std::string, std::map<std::string, double> > scale_linear_map;

  std::map<std::string, int> axis_angular_map;
  std::map<std::string, std::map<std::string, double> > scale_angular_map;

  bool sent_disable_msg;
};

/**
 * Constructs TeleopTwistJoy.
 * \param node Node to use for parameters, publisher and subscriber.
 */
TeleopTwistJoy::TeleopTwistJoy(rclcpp::Node::SharedPtr node) {
  pimpl_ = new Impl;

  std::string control_mode_topic;
  node->get_parameter_or("topics/control/mode", control_mode_topic,
                         std::string("/control/mode"));
  pimpl_->control_mode_pub =
      node->create_publisher<std_msgs::msg::UInt8>(control_mode_topic, 1);
  pimpl_->cmd_vel_pub = node->create_publisher<geometry_msgs::msg::Twist>("cmd_vel", 1);
  pimpl_->joy_sub = node->create_subscription<sensor_msgs::msg::Joy>(
      "joy", 1,
      std::bind(&TeleopTwistJoy::Impl::joyCallback, pimpl_, std::placeholders::_1));

  node->get_parameter_or("enable_button", pimpl_->enable_button, 0);
  node->get_parameter_or("enable_turbo_button", pimpl_->enable_turbo_button, -1);
  node->get_parameter_or("stand_button", pimpl_->stand_button, -1);
  node->get_parameter_or("sit_button", pimpl_->sit_button, -1);
  node->get_parameter_or("safety_button", pimpl_->safety_button, -1);

  std::map<std::string, rclcpp::Parameter> axis_linear_params;
  node->get_node_parameters_interface()->get_parameters_by_prefix("axis_linear", axis_linear_params);
  if (!axis_linear_params.empty()) {
    for (auto& kv : axis_linear_params) {
      pimpl_->axis_linear_map[kv.first] = static_cast<int>(kv.second.as_int());
    }
  } else {
    int axis_linear = 0;
    node->get_parameter_or("axis_linear", axis_linear, 1);
    pimpl_->axis_linear_map["x"] = axis_linear;
  }

  std::map<std::string, rclcpp::Parameter> scale_linear_params;
  node->get_node_parameters_interface()->get_parameters_by_prefix("scale_linear", scale_linear_params);
  if (!scale_linear_params.empty()) {
    for (auto& kv : scale_linear_params) {
      auto pos = kv.first.find('.');
      std::string mode = kv.first.substr(0, pos);
      std::string axis = kv.first.substr(pos + 1);
      pimpl_->scale_linear_map[mode][axis] = kv.second.as_double();
    }
  } else {
    double scale_linear = 0.0;
    node->get_parameter_or("scale_linear", scale_linear, 0.5);
    pimpl_->scale_linear_map["normal"]["x"] = scale_linear;
    double scale_linear_turbo = 0.0;
    node->get_parameter_or("scale_linear_turbo", scale_linear_turbo, 1.0);
    pimpl_->scale_linear_map["turbo"]["x"] = scale_linear_turbo;
  }

  std::map<std::string, rclcpp::Parameter> axis_angular_params;
  node->get_node_parameters_interface()->get_parameters_by_prefix("axis_angular", axis_angular_params);
  if (!axis_angular_params.empty()) {
    for (auto& kv : axis_angular_params) {
      pimpl_->axis_angular_map[kv.first] = static_cast<int>(kv.second.as_int());
    }
  } else {
    int axis_angular = 0;
    node->get_parameter_or("axis_angular", axis_angular, 0);
    pimpl_->axis_angular_map["yaw"] = axis_angular;
  }

  std::map<std::string, rclcpp::Parameter> scale_angular_params;
  node->get_node_parameters_interface()->get_parameters_by_prefix("scale_angular", scale_angular_params);
  if (!scale_angular_params.empty()) {
    for (auto& kv : scale_angular_params) {
      auto pos = kv.first.find('.');
      std::string mode = kv.first.substr(0, pos);
      std::string axis = kv.first.substr(pos + 1);
      pimpl_->scale_angular_map[mode][axis] = kv.second.as_double();
    }
  } else {
    double scale_angular = 0.0;
    node->get_parameter_or("scale_angular", scale_angular, 0.5);
    pimpl_->scale_angular_map["normal"]["yaw"] = scale_angular;
    pimpl_->scale_angular_map["turbo"]["yaw"] = scale_angular;
  }

  RCLCPP_INFO(LOGGER, "Teleop enable button %i.", pimpl_->enable_button);
  RCLCPP_INFO_EXPRESSION(LOGGER, pimpl_->enable_turbo_button >= 0,
                         "Turbo on button %i.", pimpl_->enable_turbo_button);

  for (auto it = pimpl_->axis_linear_map.begin();
       it != pimpl_->axis_linear_map.end(); ++it) {
    RCLCPP_INFO(LOGGER, "Linear axis %s on %i at scale %f.",
                it->first.c_str(), it->second,
                pimpl_->scale_linear_map["normal"][it->first]);
    RCLCPP_INFO_EXPRESSION(LOGGER, pimpl_->enable_turbo_button >= 0,
                           "Turbo for linear axis %s is scale %f.",
                           it->first.c_str(),
                           pimpl_->scale_linear_map["turbo"][it->first]);
  }

  for (auto it = pimpl_->axis_angular_map.begin();
       it != pimpl_->axis_angular_map.end(); ++it) {
    RCLCPP_INFO(LOGGER, "Angular axis %s on %i at scale %f.",
                it->first.c_str(), it->second,
                pimpl_->scale_angular_map["normal"][it->first]);
    RCLCPP_INFO_EXPRESSION(LOGGER, pimpl_->enable_turbo_button >= 0,
                           "Turbo for angular axis %s is scale %f.",
                           it->first.c_str(),
                           pimpl_->scale_angular_map["turbo"][it->first]);
  }

  pimpl_->sent_disable_msg = false;
}

double getVal(const sensor_msgs::msg::Joy::ConstSharedPtr& joy_msg,
              const std::map<std::string, int>& axis_map,
              const std::map<std::string, double>& scale_map,
              const std::string& fieldname) {
  if (axis_map.find(fieldname) == axis_map.end() ||
      scale_map.find(fieldname) == scale_map.end() ||
      joy_msg->axes.size() <= static_cast<size_t>(axis_map.at(fieldname))) {
    return 0.0;
  }

  return joy_msg->axes[axis_map.at(fieldname)] * scale_map.at(fieldname);
}

void TeleopTwistJoy::Impl::sendCmdVelMsg(
    const sensor_msgs::msg::Joy::ConstSharedPtr& joy_msg, const std::string& which_map) {
  // Initializes with zeros by default.
  geometry_msgs::msg::Twist cmd_vel_msg;

  cmd_vel_msg.linear.x =
      getVal(joy_msg, axis_linear_map, scale_linear_map[which_map], "x");
  cmd_vel_msg.linear.y =
      getVal(joy_msg, axis_linear_map, scale_linear_map[which_map], "y");
  cmd_vel_msg.linear.z =
      getVal(joy_msg, axis_linear_map, scale_linear_map[which_map], "z");
  cmd_vel_msg.angular.z =
      getVal(joy_msg, axis_angular_map, scale_angular_map[which_map], "yaw");
  cmd_vel_msg.angular.y =
      getVal(joy_msg, axis_angular_map, scale_angular_map[which_map], "pitch");
  cmd_vel_msg.angular.x =
      getVal(joy_msg, axis_angular_map, scale_angular_map[which_map], "roll");

  cmd_vel_pub->publish(cmd_vel_msg);

  std_msgs::msg::UInt8 control_mode_msg;

  if (static_cast<size_t>(safety_button) < joy_msg->buttons.size() &&
      joy_msg->buttons[safety_button]) {
    control_mode_msg.data = 4;
    control_mode_pub->publish(control_mode_msg);
  } else if (static_cast<size_t>(sit_button) < joy_msg->buttons.size() &&
             joy_msg->buttons[sit_button]) {
    control_mode_msg.data = 0;
    control_mode_pub->publish(control_mode_msg);
  } else if (static_cast<size_t>(stand_button) < joy_msg->buttons.size() &&
             joy_msg->buttons[stand_button]) {
    control_mode_msg.data = 1;
    control_mode_pub->publish(control_mode_msg);
  }

  sent_disable_msg = false;
}

void TeleopTwistJoy::Impl::joyCallback(
    const sensor_msgs::msg::Joy::ConstSharedPtr& joy_msg) {
  if (enable_turbo_button >= 0 &&
      joy_msg->buttons.size() > static_cast<size_t>(enable_turbo_button) &&
      joy_msg->buttons[enable_turbo_button]) {
    sendCmdVelMsg(joy_msg, "turbo");
  } else {
    sendCmdVelMsg(joy_msg, "normal");
  }
}

}  // namespace teleop_twist_joy
