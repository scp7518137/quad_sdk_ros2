#include <rclcpp/rclcpp.hpp>

#include "contact_state_publisher.h"

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  auto node = std::make_shared<ContactStatePublisher>();
  node->init();  // Must be called after make_shared for shared_from_this()
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
