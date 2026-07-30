"""ROS2 port of remote_driver.launch."""

import os
import sys

from launch import LaunchDescription
from launch.actions import (DeclareLaunchArgument, IncludeLaunchDescription,
                            OpaqueFunction)
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import param_utils  # noqa: E402


def launch_setup(context, *args, **kwargs):
    logging = param_utils.str2bool(
        LaunchConfiguration("logging").perform(context))
    live_plot = LaunchConfiguration("live_plot").perform(context)
    dash = LaunchConfiguration("dash").perform(context)
    map_input_type = LaunchConfiguration("map_input_type").perform(context)
    world = LaunchConfiguration("world").perform(context)
    robot_type = LaunchConfiguration("robot_type").perform(context)

    launch_dir = param_utils.share_path("quad_utils", "launch")

    actions = []

    # Mapping and visualization nodes
    actions.append(IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(launch_dir, "mapping.launch.py")),
        launch_arguments={
            "input_type": map_input_type,
            "world": world,
        }.items(),
    ))

    actions.append(IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(launch_dir, "quad_visualization.launch.py")),
        launch_arguments={
            "live_plot": live_plot,
            "dash": dash,
        }.items(),
    ))

    if logging:
        actions.append(IncludeLaunchDescription(
            PythonLaunchDescriptionSource(
                os.path.join(launch_dir, "logging.launch.py"))))

    # Heartbeat node
    actions.append(Node(
        package="quad_utils",
        executable="remote_heartbeat_node",
        name="remote_heartbeat",
        output="screen",
        parameters=param_utils.global_params(robot_type),
    ))

    return actions


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument("logging", default_value="false"),
        DeclareLaunchArgument("live_plot", default_value="false"),
        DeclareLaunchArgument("dash", default_value="false"),
        DeclareLaunchArgument("map_input_type", default_value="mesh"),
        DeclareLaunchArgument("world", default_value="flat"),
        DeclareLaunchArgument("robot_type", default_value="spirit"),
        OpaqueFunction(function=launch_setup),
    ])
