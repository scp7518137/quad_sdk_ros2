#include <gtest/gtest.h>
#include <rclcpp/rclcpp.hpp>

#include "local_planner/local_footstep_planner.h"

TEST(LocalFootstepPlannerTest, testTrue) {
  LocalFootstepPlanner local_footstep_planner;
  EXPECT_EQ(1 + 1, 2);
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  rclcpp::init(argc, argv);

  int result = RUN_ALL_TESTS();
  rclcpp::shutdown();
  return result;
}
