#include <gtest/gtest.h>
#include <rclcpp/rclcpp.hpp>

#include "quad_utils/rviz_interface.h"

TEST(RVizInterfaceTest, testTrue) {
  auto node = rclcpp::Node::make_shared("test_rviz_interface");
  RVizInterface rviz_interface(node);
  EXPECT_EQ(1 + 1, 2);
}
