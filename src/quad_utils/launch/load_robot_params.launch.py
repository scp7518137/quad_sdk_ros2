"""ROS2 port of load_robot_params.launch.

ROS2 has no global parameter server, so this launch file cannot "load"
parameters for nodes started elsewhere. The equivalent functionality is
provided by param_utils.robot_params(robot_type), which every quad-sdk
launch file attaches to the nodes it starts.

This file is kept for structural compatibility and simply logs a notice.
"""

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, LogInfo


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument("robot_type", default_value="spirit"),
        LogInfo(msg=(
            "load_robot_params.launch.py: ROS2 has no global parameter "
            "server. Robot parameters are attached per-node via "
            "quad_utils/launch/param_utils.py (robot_params()).")),
    ])
