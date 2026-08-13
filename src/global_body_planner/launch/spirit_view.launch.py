"""用于在 RViz2 中查看 Spirit 40 机器人模型的轻量启动文件。

用法:
  ros2 launch global_body_planner spirit_view.launch.py
"""

import os

# ROS 2 Launch 相关模块
from launch import LaunchDescription, LaunchContext # LaunchDescription: 启动描述; LaunchContext: 启动上下文
from launch.actions import OpaqueFunction   # OpaqueFunction: 延迟执行的启动动作
from launch_ros.actions import Node # Node: ROS 2 节点启动动作

from ament_index_python.packages import get_package_share_directory  # 获取 ROS 2 包的 share 目录路径


def launch_setup(context: LaunchContext, *args, **kwargs):
    """启动设置函数: 读取 URDF 模型文件并启动相关节点。"""
    # 获取 spirit_description 包的 share 目录
    share = get_package_share_directory("spirit_description")
    # 拼接 URDF 模型文件的完整路径
    urdf_path = os.path.join(share, "urdf", "spirit.urdf")

    # 读取 URDF 文件内容，作为 robot_description 参数
    with open(urdf_path, "r") as f:
        robot_desc = f.read()

    # 返回需要启动的节点列表
    return [
        # 机器人状态发布器：解析 URDF，发布 TF 变换和机器人状态
        Node(
            package="robot_state_publisher",
            executable="robot_state_publisher",
            name="robot_state_publisher",
            parameters=[{"robot_description": robot_desc}],  # 传入 URDF 描述
        ),
        # 关节状态发布器（GUI 版）：提供可拖动的滑块来手动控制各关节角度
        Node(
            package="joint_state_publisher_gui",
            executable="joint_state_publisher_gui",
            name="joint_state_publisher",
            parameters=[{"robot_description": robot_desc}],
        ),
        # RViz2 可视化工具
        Node(
            package="rviz2",
            executable="rviz2",
            name="rviz",
        ),
    ]


def generate_launch_description():
    """ROS 2 Launch 入口函数：生成 LaunchDescription 对象。"""
    # 使用 OpaqueFunction 延迟执行 launch_setup，确保在启动上下文中运行
    return LaunchDescription([OpaqueFunction(function=launch_setup)])
