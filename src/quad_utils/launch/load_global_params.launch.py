"""ROS2 port of load_global_params.launch.

ROS2 has no global parameter server, so this launch file cannot "load"
parameters for nodes started elsewhere. The equivalent functionality is
provided by param_utils.global_params(robot_type), which every quad-sdk
launch file attaches to the nodes it starts.

This file is kept for structural compatibility and simply logs a notice.
"""

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, LogInfo


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument("robot_type", default_value="spirit"),
        DeclareLaunchArgument("load_robot_params", default_value="false"),
        LogInfo(msg=(
            "load_global_params.launch.py: ROS2 has no global parameter "
            "server. Global parameters are attached per-node via "
            "quad_utils/launch/param_utils.py (global_params()).")),
    ])
