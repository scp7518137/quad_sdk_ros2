#include <rclcpp/rclcpp.hpp>
#include "global_body_planner/global_body_planner.h"
// 导入头文件


int main(int argc, char **argv) {
  rclcpp::init(argc, argv);
  //初始化
  rclcpp::NodeOptions options;
  options.allow_undeclared_parameters(true);  // 允许节点读取没有用 declare_parameter() 预先声明的参数。
  options.automatically_declare_parameters_from_overrides(true);  // 允许节点自动声明参数，这样可以在启动时从命令行或 launch 文件中传递参数，而不需要在代码中显式声明它们。
  //调用rclcpp::NodeOptions类的成员函数，设置两个参数
  auto node = std::make_shared<rclcpp::Node>("global_body_planner", options);
  //创建"global_body_planner"共享指针，填入options
  GlobalBodyPlanner global_body_planner(node);
  global_body_planner.spin();

  rclcpp::shutdown();
  return 0;
}
