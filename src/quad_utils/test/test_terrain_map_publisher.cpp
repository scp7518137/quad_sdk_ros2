#include <gtest/gtest.h>
#include <rclcpp/rclcpp.hpp>

#include "quad_utils/terrain_map_publisher.h"

TEST(TerrainMapPublisherTest, testTrue) {
  auto node = rclcpp::Node::make_shared("test_terrain_map_publisher");
  TerrainMapPublisher terrain_map_publisher(node);
  EXPECT_EQ(1 + 1, 2);
}
