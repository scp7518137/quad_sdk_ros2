#include "robot_driver/estimators/ekf_estimator.h"

EKFEstimator::EKFEstimator() {}

void EKFEstimator::init(rclcpp::Node::SharedPtr nh) {
  nh_ = nh;
  quadKD_ = std::make_shared<quad_utils::QuadKD>(nh_);
  std::cout << "EKF Estimator Initiated" << std::endl;
}

bool EKFEstimator::updateOnce(quad_msgs::msg::RobotState& last_robot_state_msg) {
  std::cout << "EKF Estimator Updated Once" << std::endl;
  return true;
}
