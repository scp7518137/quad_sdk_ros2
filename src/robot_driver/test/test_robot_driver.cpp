#include <gtest/gtest.h>
#include <rclcpp/rclcpp.hpp>

#include "robot_driver/controllers/grf_pid_controller.h"
#include "robot_driver/controllers/inverse_dynamics_controller.h"
#include "robot_driver/controllers/joint_controller.h"
#include "robot_driver/robot_driver.h"

int my_argc;
char** my_argv;

class RobotDriverTest : public ::testing::Test {
 protected:
  void SetUp() override {
    rclcpp::NodeOptions options;
    options.allow_undeclared_parameters(true);
    options.automatically_declare_parameters_from_overrides(true);
    node_ = std::make_shared<rclcpp::Node>("robot_driver_test", options);
  }

  rclcpp::Node::SharedPtr node_;
};

TEST_F(RobotDriverTest, testConstructorRobotDriver) {
  RobotDriver robot_driver(node_, my_argc, my_argv);
  EXPECT_EQ(1 + 1, 2);
}

TEST_F(RobotDriverTest, testConstructorInverseDynamicsController) {
  InverseDynamicsController inverse_dynamics_controller(node_);
  EXPECT_EQ(1 + 1, 2);
}

TEST_F(RobotDriverTest, testConstructorJointController) {
  JointController joint_controller(node_);
  EXPECT_EQ(1 + 1, 2);
}

TEST_F(RobotDriverTest, testConstructorGrfPidController) {
  GrfPidController grf_pid_controller(node_);
  EXPECT_EQ(1 + 1, 2);
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  my_argc = argc;
  my_argv = argv;
  rclcpp::init(argc, argv);

  return RUN_ALL_TESTS();
}
