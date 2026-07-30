#include <gtest/gtest.h>
#include <rclcpp/rclcpp.hpp>

#include "global_body_planner/global_body_planner.h"

class RclcppEnvironment : public ::testing::Environment {
 public:
  void SetUp() override { rclcpp::init(0, nullptr); }
  void TearDown() override { rclcpp::shutdown(); }
};

static bool registered = []() {
  ::testing::AddGlobalTestEnvironment(new RclcppEnvironment());
  return true;
}();

TEST(GlobalBodyPlannerTest, testTrue) {
  rclcpp::NodeOptions options;
  options.allow_undeclared_parameters(true);
  auto node = std::make_shared<rclcpp::Node>("test_global_body_planner", options);
  GlobalBodyPlanner global_body_planner(node);
  EXPECT_EQ(1 + 1, 2);
}
