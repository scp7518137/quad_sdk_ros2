"""ROS2 port of quad_visualization.launch."""

import os
import sys

from launch import LaunchDescription
from launch.actions import (DeclareLaunchArgument, GroupAction,
                            IncludeLaunchDescription, OpaqueFunction)
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node, PushRosNamespace

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import param_utils  # noqa: E402


def launch_setup(context, *args, **kwargs):
    robot_type = LaunchConfiguration("robot_type").perform(context)
    multiple_robots = param_utils.str2bool(
        LaunchConfiguration("multiple_robots").perform(context))
    live_plot = param_utils.str2bool(
        LaunchConfiguration("live_plot").perform(context))
    dash = param_utils.str2bool(LaunchConfiguration("dash").perform(context))
    namespace_1 = LaunchConfiguration("namespace_1").perform(context)
    namespace_2 = LaunchConfiguration("namespace_2").perform(context)

    launch_dir = param_utils.share_path("quad_utils", "launch")

    def viz_group(ns):
        return GroupAction([
            PushRosNamespace(ns),
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource(
                    os.path.join(launch_dir, "visualization_plugins.launch.py")),
                launch_arguments={
                    "namespace": ns,
                    "robot_type": robot_type,
                }.items(),
            ),
        ])

    actions = [viz_group(namespace_1)]

    if multiple_robots:
        actions.append(viz_group(namespace_2))
        # Users can add more robots here

    # RViz
    actions.append(Node(
        package="rviz2",
        executable="rviz2",
        name="rviz",
        arguments=["-d", param_utils.share_path(
            "quad_utils", "rviz", "quad_viewer.rviz")],
    ))

    # Live plotter
    if live_plot:
        actions.append(Node(
            package="plotjuggler",
            executable="plotjuggler",
            name="plotjuggler",
            arguments=["--layout", param_utils.share_path(
                "quad_utils", "config", "plotjuggler_config.xml")],
        ))

    # RQT dashboard
    if dash:
        actions.append(Node(
            package="rqt_gui",
            executable="rqt_gui",
            name="rqt_dashboard",
            respawn=False,
            output="screen",
            arguments=["--perspective-file", param_utils.share_path(
                "quad_utils", "config", "dashboard.perspective")],
        ))

    return actions


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument("robot_type", default_value="spirit"),
        DeclareLaunchArgument("multiple_robots", default_value="false"),
        DeclareLaunchArgument("live_plot", default_value="false"),
        DeclareLaunchArgument("dash", default_value="false"),
        DeclareLaunchArgument("namespace_1", default_value="robot_1"),
        DeclareLaunchArgument("namespace_2", default_value="robot_2"),
        OpaqueFunction(function=launch_setup),
    ])
