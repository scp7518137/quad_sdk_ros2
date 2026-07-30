---
name: ros2-migration-remaining-packages
overview: 按 quad_logger → nmpc_controller → local_planner 顺序，将 3 个剩余 ROS1 包迁移到 ROS2 Humble：quad_logger（Python catkin → ament_python）、nmpc_controller 和 local_planner（C++ catkin → ament_cmake）。
todos:
  - id: migrate-quad-logger
    content: "Phase 1: 迁移 quad_logger（Python catkin → ament_python），修改 package.xml、创建 setup.py/setup.cfg、更新 bag_reader.py 中 rospy 调用、删除 CMakeLists.txt"
    status: pending
  - id: verify-quad-logger
    content: 验证 quad_logger 迁移，colcon build 确认无报错
    status: pending
    dependencies:
      - migrate-quad-logger
  - id: explore-nmpc-sources
    content: 使用 [subagent:code-explorer] 探索 nmpc_controller 所有源文件，生成 ros1 API 使用点清单
    status: pending
    dependencies:
      - verify-quad-logger
  - id: migrate-nmpc-controller
    content: "Phase 2: 迁移 nmpc_controller（C++ catkin → ament_cmake），修改 package.xml、CMakeLists.txt、所有头文件和源文件中的 ros1 类型"
    status: pending
    dependencies:
      - explore-nmpc-sources
  - id: verify-nmpc-controller
    content: 验证 nmpc_controller 迁移，colcon build 确认无报错
    status: pending
    dependencies:
      - migrate-nmpc-controller
  - id: explore-local-sources
    content: 使用 [subagent:code-explorer] 探索 local_planner 所有源文件，生成 ros1 API 使用点清单
    status: pending
    dependencies:
      - verify-nmpc-controller
  - id: migrate-local-planner
    content: "Phase 3: 迁移 local_planner（C++ catkin → ament_cmake），修改 package.xml、CMakeLists.txt、发布订阅回调、节点入口"
    status: pending
    dependencies:
      - explore-local-sources
  - id: verify-all
    content: 全量验证：colcon build 构建所有已迁移包，确认零错误
    status: pending
    dependencies:
      - migrate-local-planner
---

## 用户需求
按顺序将三个剩余 ROS1 包迁移到 ROS2 Humble：quad_logger（Python） → nmpc_controller（C++） → local_planner（C++）。

## 产品概述
完成 quad-sdk 项目的完整 ROS2 迁移，使所有功能包可在 ROS2 Humble 环境下通过 colcon build 构建和运行。

## 核心功能
- **quad_logger**：bag 数据读取工具和 Qt5 可视化界面，将 catkin Python 包改造为 ament_python 包
- **nmpc_controller**：非线性模型预测控制器（NMPC），基于 IPOPT 求解器，将 catkin C++ 库改造为 ament_cmake 库
- **local_planner**：局部步态规划器，依赖 nmpc_controller 和 qpOASES，将 catkin C++ 库/节点改造为 ament_cmake 包


## 技术栈
- ROS2 Humble（Linux）
- 构建系统：ament_cmake（C++）/ ament_python（Python）
- C++ 标准：C++14/C++17
- 外部库：Ipopt、qpOASES、Eigen3、grid_map（系统包）
- 已有 ROS2 依赖：quad_msgs、quad_utils（均已迁移完成）

## 实现方案

### 整体策略
遵循已验证的 ROS2 迁移模式（参考 robot_driver/CMakeLists.txt 和 quad_utils/ros_utils.h），对三个包按依赖链顺序逐步迁移。关键原则：
1. 复用 quad_utils 已迁移的 `loadROSParam(rclcpp::Node::SharedPtr, ...)` 接口，无需改造参数加载逻辑
2. 消息类型统一使用 `quad_msgs::msg::X` 命名空间和 `.hpp` 头文件后缀
3. C++ 包统一使用 `ament_target_dependencies()` + `install(TARGETS ...)` 模式
4. 测试从 `catkin_add_gtest` 迁移到 `ament_add_gtest`

### Phase 1: quad_logger（Python → ament_python）
**高优先级改动**：
- `bag_reader.py`：`rosbag.Bag` → `rosbag2_py` 或保留 legacy rosbag（rosbag2 API 不兼容，重写成本高）；`import rospy` → `import rclpy`；`tf.transformations` → `tf_transformations` 独立包
- `read_bag.py`：Qt5 GUI 主程序，调整 import 路径
- `mouse_interface.py`：纯 matplotlib/numpy，无 ROS 依赖，无需改动

**包结构改造**：删除 `CMakeLists.txt`，创建 `setup.py` + `setup.cfg`（ament_python 标准布局），迁移 `catkin_install_python(PROGRAMS ...)` 到 `setup.py` 的 `scripts` 字段或 entry_points。

### Phase 2: nmpc_controller（C++ → ament_cmake）
**头文件改造**：
- `#include <ros/ros.h>` → `#include <rclcpp/rclcpp.hpp>`
- `quad_msgs/GRFArray.h` → `quad_msgs/msg/grf_array.hpp`（snake_case）
- `quad_msgs/LegCommand.h` → `quad_msgs/msg/leg_command.hpp`
- `quad_msgs/MultiFootPlanDiscrete.h` → `quad_msgs/msg/multi_foot_plan_discrete.hpp`
- `quad_msgs/RobotPlan.h` → `quad_msgs/msg/robot_plan.hpp`
- `quad_msgs/RobotState.h` → `quad_msgs/msg/robot_state.hpp`
- `quad_utils/ros_utils.h` → 保持不变（已 ROS2）

**成员变量改造**：
- `ros::NodeHandle nh_` → `rclcpp::Node::SharedPtr nh_`
- 构造函数签名：`NMPCController(ros::NodeHandle &nh, int type)` → `NMPCController(rclcpp::Node::SharedPtr nh, int type)`
- `quad_utils::loadROSParam(nh_, ...)` → 接口兼容，`nh_` 改为 `SharedPtr` 即可

**CMakeLists 改造**（参考 robot_driver 模板）：
- `find_package(catkin ...)` → `find_package(ament_cmake)` + `find_package(rclcpp)` + 各依赖
- `catkin_package(...)` → 删除，改用 `ament_export_include_directories()` / `ament_export_dependencies()`
- `catkin_add_gtest` → `ament_add_gtest`（包裹在 `if(BUILD_TESTING)` 中）
- `include_directories(${catkin_INCLUDE_DIRS})` → `ament_target_dependencies(nmpc_controller rclcpp quad_msgs ...)`
- `target_link_libraries(... ${catkin_LIBRARIES})` → `ament_target_dependencies(...)` + 显式 `target_link_libraries(... ipopt ...)`
- 添加 `install(TARGETS nmpc_controller ...)` + `install(DIRECTORY include/ ...)`
- 末尾添加 `ament_package()`

### Phase 3: local_planner（C++ → ament_cmake）
与 nmpc_controller 相同模式的改造，额外处理：
- **发布/订阅**：`ros::Publisher` → `rclcpp::Publisher<...>::SharedPtr`，`ros::Subscriber` → `rclcpp::Subscription<...>::SharedPtr`
- **回调函数签名**：`const quad_msgs::GridMap::ConstPtr &msg` → `const grid_map_msgs::msg::GridMap::SharedPtr &msg`
- **节点入口**：`ros::init`/`ros::NodeHandle`/`ros::spin` → `rclcpp::init`/`rclcpp::Node::SharedPtr`/`rclcpp::spin`
- **qpOASES 链接**：保持 `find_library(QPOASES ...)` 方式，链接到目标
- **PythonLibs**：2.7 → 3（如确实不再需要则移除）
- **nmpc_controller 依赖**：`find_package(nmpc_controller REQUIRED)`，通过 `ament_target_dependencies` 链接

### 性能与可靠性注意事项
- **复杂度**：nmpc_controller 和 local_planner 的 C++ 源码约 20+ 头文件、20+ 源文件，主要工作量在头文件 include 和成员变量类型替换
- **回归风险**：`ros::Time`、`ros::Duration` 在源文件中可能散布，需全部替换为 `rclcpp::Time`/`rclcpp::Duration`
- **外部库链接**：Ipopt 和 qpOASES 为系统库，链接方式不变，确保 `/usr/local/lib` 在链接搜索路径中
- **日志宏**：`ROS_INFO/ERROR/WARN` → `RCLCPP_INFO/ERROR/WARN(nh_->get_logger(), ...)`

## 目录结构

```
quad-sdk/
├── quad_logger/                          # Phase 1 修改
│   ├── CMakeLists.txt                    # [DELETE] 不再需要，ament_python 用 setup.py
│   ├── package.xml                       # [MODIFY] catkin → ament_python，依赖更新
│   ├── setup.py                          # [NEW] ament_python 构建入口
│   ├── setup.cfg                         # [NEW] ament_python 配置
│   └── scripts/
│       ├── bag_reader.py                 # [MODIFY] rospy → rclpy / rosbag API 更新
│       ├── read_bag.py                   # [MODIFY] import 路径调整
│       └── mouse_interface.py            # [NO CHANGE] 无 ROS 依赖
├── nmpc_controller/                      # Phase 2 修改
│   ├── CMakeLists.txt                    # [MODIFY] catkin → ament_cmake 全面改造
│   ├── package.xml                       # [MODIFY] catkin → ament_cmake，依赖更新
│   ├── include/nmpc_controller/
│   │   ├── nmpc_controller.h            # [MODIFY] ros::NodeHandle → SharedPtr，消息头文件路径
│   │   └── *.h                          # [MODIFY] 其余头文件中 ros1 类型替换
│   ├── src/
│   │   ├── nmpc_controller.cpp          # [MODIFY] 构造函数签名，参数加载
│   │   └── *.cpp                        # [MODIFY] 其余源文件 ros1 API 替换
│   └── test/
│       └── test_*.cpp                    # [MODIFY] 测试宏和 ros1 API 替换
├── local_planner/                        # Phase 3 修改
│   ├── CMakeLists.txt                    # [MODIFY] catkin → ament_cmake 全面改造
│   ├── package.xml                       # [MODIFY] catkin → ament_cmake，依赖更新
│   ├── include/local_planner/
│   │   ├── local_planner.h              # [MODIFY] ros::NodeHandle/Publisher/Subscriber → rclcpp 类型
│   │   └── *.h                          # [MODIFY] 其余头文件 ros1 类型替换
│   ├── src/
│   │   ├── local_planner.cpp            # [MODIFY] 发布订阅、回调、ros1 API 替换
│   │   ├── local_footstep_planner.cpp   # [MODIFY] ros1 API 替换
│   │   └── local_planner_node.cpp       # [MODIFY] ros::init → rclcpp::init，节点创建方式
│   └── test/
│       └── test_*.cpp                    # [MODIFY] 测试宏和 ros1 API 替换
```

## 关键代码结构

### nmpc_controller 构造函数签名变更
```cpp
// ROS1 原始
NMPCController(ros::NodeHandle &nh, int robot_id);
// 成员: ros::NodeHandle nh_;

// ROS2 迁移后
NMPCController(rclcpp::Node::SharedPtr nh, int robot_id);
// 成员: rclcpp::Node::SharedPtr nh_;
// 参数加载: quad_utils::loadROSParam(nh_, ...) 接口兼容，无需改动
```

### local_planner 发布订阅类型变更
```cpp
// ROS1 原始
ros::Subscriber terrain_map_sub_;
ros::Publisher local_plan_pub_;
void terrainMapCallback(const grid_map_msgs::GridMap::ConstPtr &msg);

// ROS2 迁移后
rclcpp::Subscription<grid_map_msgs::msg::GridMap>::SharedPtr terrain_map_sub_;
rclcpp::Publisher<quad_msgs::msg::RobotPlan>::SharedPtr local_plan_pub_;
void terrainMapCallback(const grid_map_msgs::msg::GridMap::SharedPtr &msg);
```


## 子代理
- **code-explorer**
  - 用途：在 Phase 2 和 Phase 3 中，探索 nmpc_controller 和 local_planner 的所有源文件，定位所有 `ros::` 类型使用点，生成完整的替换清单
  - 预期结果：列出所有需要修改的文件和行号，包括 `ros::NodeHandle`、`ros::Time`、`ros::Duration`、`ROS_INFO/WARN/ERROR`、`ros::Publisher`、`ros::Subscriber`、`ros::spin` 等 API 使用点
