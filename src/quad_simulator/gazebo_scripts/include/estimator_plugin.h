#ifndef GAZEBO_SPIRIT_ESTIMATOR_PLUGIN
#define GAZEBO_SPIRIT_ESTIMATOR_PLUGIN

#include <quad_msgs/msg/robot_state.hpp>
#include <quad_utils/math_utils.h>
#include <quad_utils/ros_utils.h>
#include <rclcpp/rclcpp.hpp>

#include <functional>
#include <gazebo/common/Plugin.hh>
#include <gazebo/common/UpdateInfo.hh>
#include <gazebo/common/common.hh>
#include <gazebo/gazebo.hh>
#include <gazebo/physics/physics.hh>
#include <ignition/math/Vector3.hh>
#include <memory>
#include <string>

namespace gazebo {

class QuadEstimatorGroundTruth : public ModelPlugin {
 public:
  QuadEstimatorGroundTruth();
  ~QuadEstimatorGroundTruth();

  void Load(physics::ModelPtr _parent, sdf::ElementPtr _sdf);
  void OnUpdate();

 private:
  double update_rate_;
  common::Time last_time_;

  rclcpp::Node::SharedPtr ros_node_;
  rclcpp::Publisher<quad_msgs::msg::RobotState>::SharedPtr
      ground_truth_state_pub_;
  rclcpp::Publisher<quad_msgs::msg::RobotState>::SharedPtr
      ground_truth_state_body_frame_pub_;

  physics::ModelPtr model_;
  event::ConnectionPtr updateConnection_;

  std::shared_ptr<quad_utils::QuadKD> quadKD_;
};

GZ_REGISTER_MODEL_PLUGIN(QuadEstimatorGroundTruth)

}  // namespace gazebo
#endif
