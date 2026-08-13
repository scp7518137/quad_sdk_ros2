# Launch 文件功能汇总

> 生成时间: 2026-08-04
> 项目: quad-sdk (ROS2 Humble)
> 共 19 个 launch 文件，全部为 ROS2 Python launch 格式

---

## 顶层入口 Launch

| Launch 文件 | 启动的节点 | 启动的软件 | 功能描述 |
|-------------|-----------|-----------|----------|
| **quad_gazebo.launch.py** | 包含: mapping.launch + quad_spawn.launch + quad_visualization.launch (per-robot 命名空间) | **Gazebo** (gazebo_ros 服务器+GUI) | ⭐ 仿真总入口。启动 Gazebo 物理仿真环境 (ODE 物理引擎)、加载 world 文件、批量生成机器人、启动可视化和规划。支持多机器人 (PushRosNamespace)。 |
| **remote_driver.launch.py** | remote_heartbeat_node; 包含: mapping.launch + quad_visualization.launch + logging.launch (条件) | RViz2, PlotJuggler, RQT | 远程/离线操作入口。启动地形建图、可视化、数据记录。供远程监控站使用。 |
| **quad_plan.launch.py** | 包含: planning.launch (per-robot 命名空间) | — | 多机器人规划入口。为每个机器人启动独立命名空间内的规划管线 (global/locel planner + teleop)。支持 goal_state 覆盖。 |

---

## 中层组合 Launch

| Launch 文件 | 启动的节点 | 启动的软件 | 功能描述 |
|-------------|-----------|-----------|----------|
| **quad_spawn.launch.py** | `gazebo_ros/spawn_entity.py` (spawn SDF 模型), `controller_manager/spawner` (ros2_control joint_controller + joint_state_controller), `gazebo_scripts/contact_state_publisher_node`; 包含: robot_driver.launch | — | 仿真生成入口：在 Gazebo 中生成机器人 SDF 模型，启动 ros2_control 关节控制器，启动接触状态发布和机器人驱动。 |
| **quad_visualization.launch.py** | `rviz2/rviz2`, `plotjuggler/plotjuggler` (条件), `rqt_gui/rqt_gui` (条件); 包含: visualization_plugins.launch (per-robot) | **RViz2**, **PlotJuggler**, **RQT** | 可视化总入口：启动 RViz2 3D 可视化、PlotJuggler 时序绘图、RQT 面板、每个机器人的 robot_state_publisher 和 rviz_interface。配置文件: quad_viewer.rviz, plotjuggler_config.xml, dashboard.perspective。 |
| **planning.launch.py** | `global_body_planner_node` (条件: reference=='gbpl'), `teleop_twist_keyboard` (条件, xterm 终端中), `local_planner_node`, `trajectory_publisher_node`; 包含: teleop.launch (条件), logging.launch (条件) | — | 规划管线：全局躯干规划器 → 局部规划器 → 轨迹发布器。支持键盘/手柄遥操作输入。参数: leaping, use_twist_input, adaptive_complexity, goal_state。重映射: start_state→state/ground_truth, goal_state→clicked_point。 |
| **mapping.launch.py** | `terrain_map_publisher_node` (条件: input_type=='grid'), `mesh_to_grid_map_node` (条件: input_type=='mesh'), `grid_map_visualization` (grid_map_visualization/grid_map_visualization), `filters_demo` (grid_map_demos/filters_demo) | — | 地形建图管线：支持 grid (PCD) 和 mesh (STL/OBJ) 两种输入类型，发布 grid_map 地形数据并可视化。配置: grid_map_visualization.yaml, filter_chain.yaml。 |

---

## 叶子 Launch

| Launch 文件 | 启动的节点 | 启动的软件 | 功能描述 |
|-------------|-----------|-----------|----------|
| **robot_driver.launch.py** | `robot_driver_node`; 包含: mocap.launch (条件), logging.launch (条件) | — | 单机器人驱动节点启动 + 可选的动捕和数据记录。参数: controller (机器人控制器类型), is_hardware (真实硬件/仿真)。 |
| **visualization_plugins.launch.py** | `robot_state_publisher` ×2 (trajectory 和 ground_truth 命名空间), `rviz_interface_node` | — | 可视化插件：为每个机器人启动两个 robot_state_publisher (规划/真值) 和一个 RViz 可视化接口节点。发布 TF 和 robot_description。 |
| **mocap.launch.py** | `mocap_node` | — | OptiTrack 动捕节点启动。配置: mocap_optitrack/config/mocap.yaml。`on_exit=Shutdown()` 确保节点退出时终止整个 launch。 |
| **logging.launch.py** | — (纯进程) | `ros2 bag record` ×2 | 数据记录：同时启动两个 ros2 bag 录制进程 (固定名称 + 时间戳归档)。记录 16 个命名空间话题 + /terrain_map。 |
| **load_global_params.launch.py** | — (空壳) | — | ⚠️ 兼容占位：声明 2 个 launch 参数后仅打印提示信息。ROS2 无全局参数服务器，功能已被 `param_utils.global_params()` 替代。 |
| **load_robot_params.launch.py** | — (空壳) | — | ⚠️ 兼容占位：声明 1 个 launch 参数后仅打印提示信息。ROS2 无全局参数服务器，功能已被 `param_utils.robot_params()` 替代。 |

---

## 外部独立 Launch

| Launch 文件 | 启动的节点 | 启动的软件 | 功能描述 |
|-------------|-----------|-----------|----------|
| **a1_rviz.launch.py** (a1_description) | `joint_state_publisher_gui`, `robot_state_publisher`, `rviz2/rviz2` | **RViz2** | A1 机器人模型预览：加载 A1 URDF/xacro、启动 joint_state_publisher_gui (可拖拽关节)、robot_state_publisher (发布 TF)、RViz2 显示。配置: check_joint.rviz, publish_frequency=1000。 |
| **grid_map_pcl_loader_node.launch.py** (grid_map_pcl) | `grid_map_pcl_loader_node` | — | PCD 加载：从 PCD 点云文件生成 grid_map。配置: config/parameters.yaml。使用 ROS2 系统包。 |
| **mocap.launch.py** (mocap_optitrack) | `mocap_node` | — | OptiTrack 动捕独立启动 (与 quad_utils/mocap.launch.py 功能相同)。`on_exit=Shutdown()`。 |
| **teleop.launch.py** (teleop_twist_joy) | `joy/joy_node`, `teleop_twist_joy/teleop_node` | — | 手柄遥操作：启动 Linux joystick 驱动节点和 teleop_twist_joy 转换节点。配置: config/<joy_config>.config.yaml (默认 ps3)。参数: dev=/dev/input/js0, deadzone=0.3。 |
| **example.launch.py** (global_body_planner) | `terrain_map_publisher_node`, `grid_map_demos/image_publisher.py`, `global_body_planner_node`, `grid_map_visualization`, `rviz_interface_node`, `rviz2/rviz2` | **RViz2** | 全局规划器示例：从 PNG 图像生成地形图，运行全局躯干规划器，RViz2 可视化。配置: example_with_planner_config.terrain.rviz, data/slope.png。参数: global_params('spirit')。 |
| **diagnostics.launch.py** (global_body_planner) | 同上例 + 加载 diagnostics.yaml (条件) | **RViz2** | 全局规划器诊断：与 example.launch.py 相同节点集，额外加载诊断配置用于调试。 |

---

## Launch 层级关系

```
quad_gazebo.launch.py          ← 仿真总入口
├── Gazebo (gazebo_ros)
├── mapping.launch.py
│   ├── terrain_map_publisher_node / mesh_to_grid_map_node
│   ├── grid_map_visualization
│   └── filters_demo
├── quad_spawn.launch.py       ← per-robot
│   ├── spawn_entity.py (spawn SDF)
│   ├── ros2_control spawner
|   ├── contact_state_publisher_node
│   └── robot_driver.launch.py
│       ├── robot_driver_node
│       ├── mocap.launch.py → mocap_node
│       └── logging.launch.py → ros2 bag record
└── quad_visualization.launch.py ← per-robot
    ├── rviz2, plotjuggler, rqt_gui
    └── visualization_plugins.launch.py
        ├── robot_state_publisher ×2
        └── rviz_interface_node

quad_plan.launch.py            ← 多机器人规划入口
└── planning.launch.py         ← per-robot
    ├── global_body_planner_node
    ├── local_planner_node
    ├── trajectory_publisher_node
    ├── teleop.launch.py → joy_node + teleop_node
    └── teleop_twist_keyboard (xterm)

remote_driver.launch.py        ← 远程操作入口
├── remote_heartbeat_node
├── mapping.launch.py
├── quad_visualization.launch.py
└── logging.launch.py
```
