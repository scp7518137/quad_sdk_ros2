"""ROS2 port of quad_plan.launch."""

import os
import sys

from launch import LaunchDescription
from launch.actions import (DeclareLaunchArgument, GroupAction,
                            IncludeLaunchDescription, OpaqueFunction)
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import PushRosNamespace

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import param_utils  # noqa: E402


def launch_setup(context, *args, **kwargs):
    reference = LaunchConfiguration("reference").perform(context)
    multiple_robots = param_utils.str2bool(
        LaunchConfiguration("multiple_robots").perform(context))
    logging = LaunchConfiguration("logging").perform(context)
    twist_input = LaunchConfiguration("twist_input").perform(context)
    leaping = LaunchConfiguration("leaping").perform(context)
    ac = LaunchConfiguration("ac").perform(context)
    robot_type = LaunchConfiguration("robot_type").perform(context)
    namespace_1 = LaunchConfiguration("namespace_1").perform(context)
    namespace_2 = LaunchConfiguration("namespace_2").perform(context)
    use_sim_time = LaunchConfiguration("use_sim_time").perform(context)

    planning_launch = os.path.join(
        param_utils.share_path("quad_utils", "launch"), "planning.launch.py")

    def planning_group(ns, goal_state=""):
        return GroupAction([
            PushRosNamespace(ns),
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource(planning_launch),
                launch_arguments={
                    "reference": reference,
                    "logging": logging,
                    "twist_input": twist_input,
                    "leaping": leaping,
                    "ac": ac,
                    "robot_type": robot_type,
                    "namespace": ns,
                    "goal_state": goal_state,
                    "use_sim_time": use_sim_time,
                }.items(),
            ),
        ])

    actions = [planning_group(namespace_1)]

    if multiple_robots:
        actions.append(planning_group(namespace_2, goal_state="[5.0, 1.0]"))
        # Add more robots here

    return actions


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument("reference", default_value="gbpl"),
        DeclareLaunchArgument("multiple_robots", default_value="false"),
        DeclareLaunchArgument("logging", default_value="false"),
        DeclareLaunchArgument("twist_input", default_value="none"),
        DeclareLaunchArgument("leaping", default_value="true"),
        DeclareLaunchArgument("ac", default_value="false"),
        DeclareLaunchArgument("robot_type", default_value="spirit"),
        DeclareLaunchArgument("namespace_1", default_value="robot_1"),
        DeclareLaunchArgument("namespace_2", default_value="robot_2"),
        DeclareLaunchArgument("use_sim_time", default_value="false"),
        OpaqueFunction(function=launch_setup),
    ])
