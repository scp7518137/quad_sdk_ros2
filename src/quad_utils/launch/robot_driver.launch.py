"""ROS2 port of robot_driver.launch."""

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
    robot_type = LaunchConfiguration("robot_type").perform(context)
    mocap = param_utils.str2bool(
        LaunchConfiguration("mocap").perform(context))
    logging = param_utils.str2bool(
        LaunchConfiguration("logging").perform(context))
    controller = LaunchConfiguration("controller").perform(context)
    is_hardware = param_utils.str2bool(
        LaunchConfiguration("is_hardware").perform(context))
    use_sim_time = param_utils.str2bool(
        LaunchConfiguration("use_sim_time").perform(context))

    launch_dir = param_utils.share_path("quad_utils", "launch")

    actions = []

    # Robot driver node (ROS1 private params controller / is_hardware)
    actions.append(Node(
        package="robot_driver",
        executable="robot_driver_node",
        name="robot_driver",
        output="screen",
        parameters=param_utils.global_params(
            robot_type, use_sim_time=use_sim_time) + [{
                "robot_driver.controller": controller,
                "robot_driver.is_hardware": is_hardware,
            }],
    ))

    # Launch the mocap node if specified
    if mocap:
        actions.append(IncludeLaunchDescription(
            PythonLaunchDescriptionSource(
                os.path.join(launch_dir, "mocap.launch.py"))))

    # Record into bag if specified
    if logging:
        actions.append(IncludeLaunchDescription(
            PythonLaunchDescriptionSource(
                os.path.join(launch_dir, "logging.launch.py"))))

    return actions


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument("robot_type", default_value="spirit"),
        DeclareLaunchArgument("mocap", default_value="true"),
        DeclareLaunchArgument("logging", default_value="false"),
        DeclareLaunchArgument("controller", default_value="inverse_dynamics"),
        DeclareLaunchArgument("is_hardware", default_value="true"),
        DeclareLaunchArgument("use_sim_time", default_value="false"),
        OpaqueFunction(function=launch_setup),
    ])
