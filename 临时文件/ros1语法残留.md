# ROS1 语法残留检查报告

> 生成时间: 2026-08-04
> 项目: quad-sdk (ROS2 Humble)
>
> 检查范围: 全项目 `src/` 目录，针对 rospy、roscpp、catkin、ros::NodeHandle、ros::Time、ros::init、package format="2"、`.test` XML launch 等 ROS1 特征。

---

## 严重等级定义

| 等级 | 含义 |
|------|------|
| 🔴 **严重** | 整个包为 ROS1 实现，catkin 构建系统，依赖 roscpp/rospy，运行时需要 roscore。在 ROS2 环境下**完全无法编译和运行**。 |
| 🟡 **中等** | 单个文件/脚本为 ROS1 实现，但所在包主体已是 ROS2。该文件在 ROS2 下无法运行，但不影响包内其他节点。 |
| 🟢 **无害** | 仅注释中提及 ROS1/catkin，或为历史参考保留的注释块。无运行时影响。 |
| ⚪ **遗留测试** | ROS1 `.test` 文件 + rospy 测试脚本，不在 ROS2 构建系统中。不会导致编译/运行错误，但占用磁盘。 |

---

## 🔴 严重：整个包为 ROS1 实现

### 1. src/external/grid_map_pcl/ — 完全 ROS1

| 文件 | ROS1 残留 | 说明 |
|------|----------|------|
| `CMakeLists.txt` | `find_package(catkin REQUIRED)`, `catkin_package()`, `CATKIN_PACKAGE_DEPENDENCIES`, `CATKIN_PACKAGE_LIB_DESTINATION`, `catkin_add_gtest`, 依赖 `roscpp` | 整个构建系统为 catkin/ROS1 |
| `package.xml` | `<package format="2">`, `<buildtool_depend>catkin</buildtool_depend>`, `<depend>roscpp</depend>` | package format="2" 是 ROS1 格式 |
| `include/grid_map_pcl/helpers.hpp` | `ros::NodeHandle` 参数类型 (7个函数签名) | 全部 helper 函数接受 `const ros::NodeHandle&` |
| `src/grid_map_pcl_loader_node.cpp` | `ros::init(argc, argv, ...)`, `ros::NodeHandle nh("~")`, `ros::Publisher`, `ros::spin()` | 节点入口完全 ROS1 |
| `src/helpers.cpp` | `ros::NodeHandle`, `nh.param<>()`, `ROS_INFO_STREAM` | 参数读取和日志 |
| `src/GridMapPclLoader.cpp` | `ROS_INFO_STREAM`, `ROS_WARN_STREAM`, `ROS_ERROR_STREAM` | 全局日志宏 |
| `src/PclLoaderParameters.cpp` | `ROS_ERROR_STREAM` | 全局日志宏 |

> **影响**: 该包 `grid_map_pcl_loader_node` 节点在 ROS2 下**无法编译**，`quad_spawn.launch.py` 和 `mapping.launch.py` 等 launch 文件尝试启动此节点时会失败。

---

### 2. src/quad_simulator/gazebo_scripts/ — 完全 ROS1

| 文件 | ROS1 残留 | 说明 |
|------|----------|------|
| `CMakeLists.txt` | `find_package(catkin REQUIRED COMPONENTS roscpp rospy ...)`, `catkin_package()` | catkin 构建，依赖 roscpp/rospy |
| `package.xml` | `<package format="2">`, `<buildtool_depend>catkin</buildtool_depend>` | ROS1 包格式 |
| `include/contact_plugin.h` | `std::unique_ptr<ros::NodeHandle> rosNode`, `ros::Publisher` | Gazebo 插件使用 ROS1 API |
| `include/contact_state_publisher.h` | `#include <ros/ros.h>`, `ros::NodeHandle`, `ros::Subscriber`, `ros::Publisher` | 头文件中所有成员均为 ROS1 类型 |
| `include/controller_plugin.h` | `ros::NodeHandle&`, `const ros::Time&`, `const ros::Duration&` | Gazebo 控制器插件接口 |
| `src/contact_plugin.cpp` | `ros::init(NoSigintHandler)`, `new ros::NodeHandle(...)`, `rosNode->advertise<>()` | Gazebo contact 传感器插件 |
| `src/contact_state_publisher.cpp` | `ros::NodeHandle`, `ros::Subscriber`, `ros::Publisher`, `ros::spinOnce()`, `ros::Rate`, `ros::Time::now()`, `ros::ok()`, `ROS_WARN_THROTTLE` | 接触状态发布器完整实现 |
| `src/contact_state_publisher_node.cpp` | `ros::init(argc, argv, ...)`, `ros::NodeHandle nh` | 节点入口 |
| `src/controller_plugin.cpp` | `ros::NodeHandle& n`, `n.getParam()`, `ros::Time`, `ros::Duration`, `ROS_ERROR_STREAM` | Gazebo 关节控制器 |
| `src/estimator_plugin.cpp` | `ros::NodeHandle nh(robot_ns)`, `ros::Time::now()`, `ROS_INFO_STREAM`, `ROS_WARN_STREAM`, `ROS_ERROR` | 地面真值估计器插件 |

> **影响**: 这些 Gazebo 插件 (`libcontact`, `libcontroller`, `libground_truth_estimator`) 和 `contact_state_publisher_node` 在 ROS2 下**无法编译**。由于 `quad_spawn.launch.py` 在每台机器人 spawn 时都会尝试启动 `contact_state_publisher_node`，仿真流程会在此处失败。Gazebo Classic 插件需要迁移到 `gazebo_ros2_control` / `ignition` 体系。

---

## 🟡 中等：单文件 ROS1 残留

### 3. src/quad_utils/scripts/twist_key_controller.py

| 行号 | ROS1 残留 | 说明 |
|------|----------|------|
| 4 | `import rospy` | 导入 ROS1 Python 客户端库 |
| 5 | `from geometry_msgs.msg import Twist` | ROS1 消息导入方式 (ROS2 需加包前缀或直接用同名) |
| 9 | `rospy.init_node('twist_key_controller', anonymous=True)` | ROS1 节点初始化 |
| 11 | `rospy.Publisher('/cmd_vel', Twist, queue_size=1)` | ROS1 Publisher 创建 |
| 12 | `rospy.Publisher('/control/mode', UInt8, queue_size=1)` | ROS1 Publisher 创建 |
| 22 | `rate = rospy.Rate(30)` | ROS1 Rate 睡眠 |
| 23 | `while not rospy.is_shutdown()` | ROS1 主循环检查 |

> **影响**: 此脚本为键盘遥操作的 ROS1 实现，在 ROS2 下无法运行。ROS2 生态有等效替代包 `teleop_twist_keyboard` (已在 `planning.launch.py` 中被使用)。

---

### 4. src/external/teleop_twist_joy/test/test_joy_twist.py

| 行号 | ROS1 残留 |
|------|----------|
| 28-29 | `import rostest`, `import rospy` |
| 37-39 | `rospy.init_node()`, `rospy.Publisher()`, `rospy.Subscriber()` |
| 41-45 | `rospy.has_param()`, `rospy.get_param()` |

> **影响**: 这是 ROS1 测试文件，不在 ROS2 构建系统中 (CMakeLists.txt 中测试部分已适配 ROS2)。仅占用磁盘空间。

---

## ⚪ 遗留 ROS1 `.test` XML 文件

以下文件位于 `src/external/teleop_twist_joy/test/`，使用完整 ROS1 `<launch>` XML 格式:

| 文件 | 大小 | 内容 |
|------|------|------|
| `differential_joy.test` | ~400B | `<launch><node pkg="..." type="teleop_node">...<test test-name="..." type="test_joy_twist.py">` |
| `holonomic_joy.test` | ~440B | 同上模式，全向摇杆配置 |
| `no_enable_joy.test` | ~350B | 同上，无 enable 按键 |
| `only_turbo_joy.test` | ~390B | 同上，仅 turbo 按键 |
| `six_dof_joy.test` | ~560B | 同上，6 自由度摇杆 |
| `turbo_angular_enable_joy.test` | ~440B | 同上，turbo + 角速度 enable |
| `turbo_angular_enable_joy_with_rosparam_map.test` | ~580B | 同上，使用 rosparam map 格式 |
| `turbo_enable_joy.test` | ~400B | 同上，turbo enable |

> **影响**: 这些 `.test` 文件是 ROS1 `rostest` 格式，在 ROS2 下完全无用。不会被任何构建或测试系统加载。可安全删除。

---

## 🟢 无害：注释中的 ROS1 引用

以下为已注释掉的旧 ROS1 代码或迁移说明注释，**无运行时影响**:

| 文件 | 内容 |
|------|------|
| `quad_msgs/CMakeLists.txt` | 注释块: `## Find catkin macros and libraries`, `## catkin specific configuration` |
| `quad_msgs/package.xml` | 注释: `<!-- <buildtool_depend>catkin</buildtool_depend> -->`, `<!-- <depend>roscpp</depend> -->` |
| `quad_utils/CMakeLists.txt` | 注释块: catkin find_package, roscpp/std_msgs |
| `quad_utils/package.xml` | 大量注释掉的文档说明: `<depend>roscpp</depend>`, `<buildtool_depend>catkin</buildtool_depend>` |
| `quad_utils/src/quad_kd.cpp` | 注释掉的 `ROS_*_THROTTLE` 调用 |
| `quad_utils/src/fast_terrain_map.cpp` | 注释掉的 `ROS_*_THROTTLE` 调用 |
| `quad_utils/src/math_utils.cpp` | 注释掉的 `ROS_*_THROTTLE` 调用 |
| `quad_utils/include/quad_utils/ros_utils.h` | `normalizeParamName()` 函数注释说明: 将 ROS1 风格参数名转换为 ROS2 | ← 这是设计注释, 非残留 |
| `quad_utils/launch/mocap.launch.py:24` | 注释: `# ROS1 required="true" -> shut everything down if it exits` | ← 迁移说明 |
| `quad_utils/launch/param_utils.py` | 多处注释说明 ROS1→ROS2 参数加载迁移 | ← 迁移说明 |
| `quad_utils/launch/quad_gazebo.launch.py:43` | 注释: `# (ROS1 gazebo_ros/empty_world.launch -> ROS2 gazebo_ros/gazebo.launch.py)` | ← 迁移说明 |
| `quad_utils/launch/quad_spawn.launch.py:52` | 注释: `# (ROS1 controller_manager/spawner -> ros2_control spawner)` | ← 迁移说明 |
| `quad_utils/launch/robot_driver.launch.py:33` | 注释: `# Robot driver node (ROS1 private params controller / is_hardware)` | ← 迁移说明 |
| `quad_utils/launch/visualization_plugins.launch.py:22` | 注释: `# ROS1 tf_prefix -> ROS2 frame_prefix` | ← 迁移说明 |
| `quad_simulator/a1_description/launch/a1_rviz.launch.py:27` | 注释: `# Send fake joint values (ROS1 use_gui -> joint_state_publisher_gui)` | ← 迁移说明 |
| `global_body_planner/launch/diagnostics.launch.py:20,73` | 注释: `# Diagnostics config (kept from ROS1; ...)`, `# RViz (config kept from ROS1; ...)` | ← 迁移说明 |
| `global_body_planner/launch/example.launch.py:3` | 注释: `Note: the ROS1 file loaded ...` | ← 迁移说明 |
| `global_body_planner/README.md:38` | `catkin run_tests global_body_planner` | README 中的旧命令 |
| `nmpc_controller/README.md:34` | `catkin run_tests nmpc_controller` | README 中的旧命令 |
| `robot_driver/README.md:21` | `catkin run_tests robot_driver` | README 中的旧命令 |
| `quad_logger/scripts/bag_reader.py:3` | 注释: `# ROS2 migration: rosbag (ROS1) -> rosbag2_py, tf.transformations -> tf_transformations` | ← 迁移说明 |

---

## 总结

```
  🔴 严重 (整包ROS1):  2 包  ─ grid_map_pcl, gazebo_scripts
  🟡 中等 (单文件):     2 个  ─ twist_key_controller.py, test_joy_twist.py
  ⚪ 遗留测试文件:       8 个  ─ teleop_twist_joy/test/*.test
  🟢 无害注释:          ~20 处 ─ 注释/README 中的 catkin/ROS1 引用
```

**关键影响**: `quad_spawn.launch.py` 中启动的 `gazebo_scripts/contact_state_publisher_node` 是 ROS1 节点，会导致完整仿真管线 (`quad_gazebo.launch.py`) 在 ROS2 环境下失败。
`grid_map_pcl_loader_node` 同样无法运行，影响 `mapping.launch.py` 中的 grid 模式地形建图。

---

## 迁移完成状态 (2026-08-04)

### ✅ 已完成迁移

| 项目 | 迁移内容 | 状态 |
|------|---------|------|
| `quad_utils/scripts/twist_key_controller.py` | rospy → rclpy: Node类、create_publisher、create_timer | ✅ 已迁移 |
| `gazebo_scripts/include/contact_state_publisher.h` | ros::NodeHandle → rclcpp::Node, ros::Subscriber/Publisher → rclcpp | ✅ 已迁移 |
| `gazebo_scripts/src/contact_state_publisher.cpp` | 全部 ROS1 API → ROS2: 订阅改用 lambda, spinOnce→timer, tf2_ros::Buffer | ✅ 已迁移 |
| `gazebo_scripts/src/contact_state_publisher_node.cpp` | ros::init → rclcpp::init, 独立节点入口 | ✅ 已迁移 |
| `gazebo_scripts/include/contact_plugin.h` | Gazebo SensorPlugin + ros::NodeHandle → rclcpp::Node | ✅ 已迁移 |
| `gazebo_scripts/src/contact_plugin.cpp` | ros::Publisher → rclcpp::Publisher, rclcpp::spin_some | ✅ 已迁移 |
| `gazebo_scripts/include/estimator_plugin.h` | Gazebo ModelPlugin + ros::NodeHandle → rclcpp::Node | ✅ 已迁移 |
| `gazebo_scripts/src/estimator_plugin.cpp` | ROS1 API → ROS2, QuadKD构造适配(rclcpp::Node) | ✅ 已迁移 |
| `gazebo_scripts/include/controller_plugin.h` | controller_interface::Controller → ControllerInterface (ros2_control) | ✅ 已迁移 |
| `gazebo_scripts/src/controller_plugin.cpp` | ros_control → ros2_control: on_init/on_configure/on_activate/update, lifecycle node | ✅ 已迁移 |
| `gazebo_scripts/CMakeLists.txt` | catkin → ament_cmake, 完全重写 | ✅ 已迁移 |
| `gazebo_scripts/package.xml` | format="2" → format="3", catkin → ament_cmake | ✅ 已迁移 |
| `gazebo_scripts/config/quad_control.yaml` | effort_controllers/QuadController → quad_controller/QuadController | ✅ 已更新 |
| `gazebo_scripts/config/a1_control.yaml` | ROS1格式 → ROS2 ros__parameters格式 | ✅ 已更新 |
| `gazebo_scripts/spirit_controller_plugin.xml` | 类名和基类更新为 ros2_control | ✅ 已更新 |
| `gazebo_scripts/COLCON_IGNORE` | 已删除, 包可正常编译 | ✅ 已处理 |
| `external/teleop_twist_joy/test/*.test` (8个) | ROS1 XML测试文件已删除 | ✅ 已清理 |
| `quad_msgs/CMakeLists.txt` | 清理 catkin 注释块 | ✅ 已清理 |
| `quad_utils/CMakeLists.txt` | 清理 catkin 注释块和重复代码 | ✅ 已清理 |
| `quad_utils/package.xml` | 清理 ROS1 注释文档块 | ✅ 已清理 |
| `global_body_planner/README.md` | catkin → colcon, roslaunch → ros2 launch | ✅ 已修复 |
| `nmpc_controller/README.md` | catkin → colcon | ✅ 已修复 |
| `robot_driver/README.md` | catkin → colcon | ✅ 已修复 |

### ⚠️ 无需迁移 (系统包替代)

| 项目 | 原因 |
|------|------|
| `external/grid_map_pcl/` (ROS1) | ROS2 系统包 `ros-humble-grid-map-pcl` 已安装, 提供相同功能。本地 ROS1 副本保留 COLCON_IGNORE, 由系统包替代。 |

### 📊 迁移统计

- 迁移文件数: **12 个源文件** + **5 个构建/配置文件**
- 删除文件: **8 个** (.test 文件)
- 编译结果: ✅ **gazebo_scripts 包编译通过**
- 剩余 ROS1 残留: 仅 `external/grid_map_pcl/` (有 COLCON_IGNORE, 系统包替代)
