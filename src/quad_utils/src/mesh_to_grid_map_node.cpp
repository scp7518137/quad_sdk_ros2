#include <rclcpp/rclcpp.hpp>

#include "quad_utils/mesh_to_grid_map_converter.hpp"

// Standard C++ entry point
int main(int argc, char** argv) {
  // Announce this program to the ROS 2 network
  rclcpp::init(argc, argv);
  rclcpp::NodeOptions options;
  options.allow_undeclared_parameters(true);
  auto node =
      std::make_shared<rclcpp::Node>("mesh_to_grid_map_node", options);

  // Creating the object to do the work.
  mesh_to_grid_map::MeshToGridMapConverter mesh_to_grid_map_converter(node);

  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
