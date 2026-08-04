#include "controller_plugin.h"

#include <angles/angles.h>
#include <pluginlib/class_list_macros.hpp>

namespace quad_controller {

QuadController::QuadController() {
  // Setup joint map (maps ros2_control joint index to leg/joint pair)
  leg_map_[0] = std::make_pair(0, 1);   // hip0
  leg_map_[1] = std::make_pair(0, 2);   // knee0
  leg_map_[2] = std::make_pair(1, 1);   // hip1
  leg_map_[3] = std::make_pair(1, 2);   // knee1
  leg_map_[4] = std::make_pair(2, 1);   // hip2
  leg_map_[5] = std::make_pair(2, 2);   // knee2
  leg_map_[6] = std::make_pair(3, 1);   // hip3
  leg_map_[7] = std::make_pair(3, 2);   // knee3
  leg_map_[8] = std::make_pair(0, 0);   // abd0
  leg_map_[9] = std::make_pair(1, 0);   // abd1
  leg_map_[10] = std::make_pair(2, 0);  // abd2
  leg_map_[11] = std::make_pair(3, 0);  // abd3

  // Torque saturation
  torque_lims_ = {21, 21, 32};
  speed_lims_ = {37.7, 37.7, 25.1};
}

QuadController::~QuadController() = default;

controller_interface::InterfaceConfiguration
QuadController::command_interface_configuration() const {
  controller_interface::InterfaceConfiguration config;
  config.type = controller_interface::interface_configuration_type::INDIVIDUAL;

  for (const auto& joint_name : joint_names_) {
    config.names.push_back(joint_name + "/effort");
  }

  return config;
}

controller_interface::InterfaceConfiguration
QuadController::state_interface_configuration() const {
  controller_interface::InterfaceConfiguration config;
  config.type = controller_interface::interface_configuration_type::INDIVIDUAL;

  for (const auto& joint_name : joint_names_) {
    config.names.push_back(joint_name + "/position");
    config.names.push_back(joint_name + "/velocity");
  }

  return config;
}

controller_interface::CallbackReturn QuadController::on_init() {
  return controller_interface::CallbackReturn::SUCCESS;
}

controller_interface::CallbackReturn QuadController::on_configure(
    const rclcpp_lifecycle::State& /*previous_state*/) {
  auto node = get_node();

  // Get joint names from parameters
  joint_names_ = auto_declare<std::vector<std::string>>("joints", {});
  n_joints_ = joint_names_.size();

  if (n_joints_ == 0) {
    RCLCPP_ERROR(node->get_logger(), "List of joint names is empty.");
    return controller_interface::CallbackReturn::ERROR;
  }

  // Get URDF
  auto robot_description = auto_declare<std::string>("robot_description", "");
  urdf::Model urdf;
  if (!urdf.initString(robot_description)) {
    RCLCPP_ERROR(node->get_logger(), "Failed to parse URDF file");
    return controller_interface::CallbackReturn::ERROR;
  }

  for (unsigned int i = 0; i < n_joints_; i++) {
    urdf::JointConstSharedPtr joint_urdf = urdf.getJoint(joint_names_[i]);
    if (!joint_urdf) {
      RCLCPP_ERROR(node->get_logger(), "Could not find joint '%s' in URDF",
                   joint_names_[i].c_str());
      return controller_interface::CallbackReturn::ERROR;
    }
    joint_urdfs_.push_back(joint_urdf);
  }

  // Initialize command buffer
  int num_legs = 4;
  commands_buffer_.writeFromNonRT(BufferType(num_legs));

  // Derive joint command topic from namespace.
  // The lifecycle node namespace is e.g. /robot_1/joint_controller.
  // We strip /joint_controller suffix and append /control/joint_command.
  std::string ns(node->get_namespace());
  std::string private_ns = "/joint_controller";
  if (ns.size() > private_ns.size() &&
      ns.compare(ns.size() - private_ns.size(), private_ns.size(),
                 private_ns) == 0) {
    ns.resize(ns.size() - private_ns.size());
  }
  if (ns.empty()) {
    ns = "/";
  }
  std::string joint_command_topic = ns + "/control/joint_command";

  // Create subscriber
  sub_command_ =
      node->create_subscription<quad_msgs::msg::LegCommandArray>(
          joint_command_topic, 1,
          std::bind(&QuadController::commandCB, this,
                    std::placeholders::_1));

  return controller_interface::CallbackReturn::SUCCESS;
}

controller_interface::CallbackReturn QuadController::on_activate(
    const rclcpp_lifecycle::State& /*previous_state*/) {
  return controller_interface::CallbackReturn::SUCCESS;
}

controller_interface::CallbackReturn QuadController::on_deactivate(
    const rclcpp_lifecycle::State& /*previous_state*/) {
  return controller_interface::CallbackReturn::SUCCESS;
}

controller_interface::return_type QuadController::update(
    const rclcpp::Time& /*time*/, const rclcpp::Duration& /*period*/) {
  BufferType& commands = *commands_buffer_.readFromRT();

  // Check if message is populated
  if (commands.empty() || commands.front().motor_commands.empty()) {
    return controller_interface::return_type::OK;
  }

  for (unsigned int i = 0; i < n_joints_; i++) {
    std::pair<int, int> ind = leg_map_[i];
    quad_msgs::msg::MotorCommand motor_command =
        commands[ind.first].motor_commands[ind.second];

    // Collect feedforward torque
    double torque_ff = motor_command.torque_ff;

    // Compute position error
    double command_position = motor_command.pos_setpoint;
    enforceJointLimits(command_position, i);
    double current_position = command_interfaces_[i].get_value();
    double kp = motor_command.kp;
    double pos_error;
    angles::shortest_angular_distance_with_large_limits(
        current_position, command_position, joint_urdfs_[i]->limits->lower,
        joint_urdfs_[i]->limits->upper, pos_error);

    // Compute velocity error (velocity is at index n_joints_ + i)
    double current_vel =
        state_interfaces_[n_joints_ + i].get_value();
    double command_vel = motor_command.vel_setpoint;
    double vel_error = command_vel - current_vel;
    double kd = motor_command.kd;

    // Collect feedback
    double torque_feedback = kp * pos_error + kd * vel_error;
    double torque_lim = torque_lims_[ind.second];
    double motor_model_ub = torque_lims_[ind.second] *
                            (1.0 - current_vel / speed_lims_[ind.second]);
    double motor_model_lb = -torque_lims_[ind.second] *
                            (1.0 - current_vel / speed_lims_[ind.second]);
    double torque_command = std::min(
        std::max(torque_feedback + torque_ff, -torque_lim), torque_lim);
    bool apply_motor_model = false;
    torque_command =
        (apply_motor_model)
            ? std::min(std::max(torque_command, motor_model_lb), motor_model_ub)
            : torque_command;

    // Update joint torque
    command_interfaces_[i].set_value(torque_command);
  }

  return controller_interface::return_type::OK;
}

void QuadController::commandCB(
    const quad_msgs::msg::LegCommandArray::ConstSharedPtr msg) {
  commands_buffer_.writeFromNonRT(msg->leg_commands);
}

void QuadController::enforceJointLimits(double& command,
                                        unsigned int index) {
  // Check that this joint has applicable limits
  if (joint_urdfs_[index]->type == urdf::Joint::REVOLUTE ||
      joint_urdfs_[index]->type == urdf::Joint::PRISMATIC) {
    if (command > joint_urdfs_[index]->limits->upper) {
      command = joint_urdfs_[index]->limits->upper;
    } else if (command < joint_urdfs_[index]->limits->lower) {
      command = joint_urdfs_[index]->limits->lower;
    }
  }
}

}  // namespace quad_controller

PLUGINLIB_EXPORT_CLASS(quad_controller::QuadController,
                       controller_interface::ControllerInterface)
