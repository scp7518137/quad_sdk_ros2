#pragma once

#include <controller_interface/controller_interface.hpp>
#include <quad_msgs/msg/leg_command.hpp>
#include <quad_msgs/msg/leg_command_array.hpp>
#include <quad_msgs/msg/motor_command.hpp>
#include <quad_utils/ros_utils.h>
#include <realtime_tools/realtime_buffer.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>
#include <urdf/model.h>

#include <angles/angles.h>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace quad_controller {

/**
 * \brief Forward command controller for quadrupeds (ros2_control).
 *
 * This class forwards the commanded efforts down to a set of joints.
 * Based on the ROS1 effort_controllers::QuadController.
 */
class QuadController : public controller_interface::ControllerInterface {
  typedef std::vector<quad_msgs::msg::LegCommand> BufferType;

 public:
  QuadController();
  ~QuadController();

  controller_interface::InterfaceConfiguration
  command_interface_configuration() const override;

  controller_interface::InterfaceConfiguration
  state_interface_configuration() const override;

  controller_interface::CallbackReturn on_init() override;

  controller_interface::CallbackReturn on_configure(
      const rclcpp_lifecycle::State& previous_state) override;

  controller_interface::CallbackReturn on_activate(
      const rclcpp_lifecycle::State& previous_state) override;

  controller_interface::CallbackReturn on_deactivate(
      const rclcpp_lifecycle::State& previous_state) override;

  controller_interface::return_type update(
      const rclcpp::Time& time, const rclcpp::Duration& period) override;

 private:
  void commandCB(
      const quad_msgs::msg::LegCommandArray::ConstSharedPtr msg);

  void enforceJointLimits(double& command, unsigned int index);

  /// Joint names
  std::vector<std::string> joint_names_;

  /// Number of joints
  unsigned int n_joints_;

  /// Realtime buffer for incoming commands
  realtime_tools::RealtimeBuffer<BufferType> commands_buffer_;

  /// Subscriber for new LegCommandArray messages
  rclcpp::Subscription<quad_msgs::msg::LegCommandArray>::SharedPtr sub_command_;

  /// Reference to urdf joints
  std::vector<urdf::JointConstSharedPtr> joint_urdfs_;

  /// Map ros2_control joint indices to leg/joint pair
  std::map<int, std::pair<int, int>> leg_map_;

  /// Torque limits for each motor
  std::vector<double> torque_lims_;
  std::vector<double> speed_lims_;
};

}  // namespace quad_controller
