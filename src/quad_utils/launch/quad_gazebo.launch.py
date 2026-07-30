"""ROS2 port of quad_gazebo.launch (top-level simulation entry point).

Note: requires the gazebo_scripts package to be built (its COLCON_IGNORE
must be removed) so that worlds/ and the controller plugins are available.
"""

import os
import sys

from launch import LaunchDescription
from launch.actions import (DeclareLaunchArgument, GroupAction,
                            IncludeLaunchDescription, OpaqueFunction)
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import PushRosNamespace, SetParameter

from ament_index_python.packages import get_package_share_directory

sys.path.insert(0, os.path.join(
    get_package_share_directory("quad_utils"), "launch"))
import param_utils  # noqa: E402


def launch_setup(context, *args, **kwargs):
    robot_type = LaunchConfiguration("robot_type").perform(context)
    multiple_robots = param_utils.str2bool(
        LaunchConfiguration("multiple_robots").perform(context))
    gui = param_utils.str2bool(LaunchConfiguration("gui").perform(context))
    paused = LaunchConfiguration("paused").perform(context)
    controller = LaunchConfiguration("controller").perform(context)
    live_plot = LaunchConfiguration("live_plot").perform(context)
    dash = LaunchConfiguration("dash").perform(context)
    world = LaunchConfiguration("world").perform(context)

    launch_dir = param_utils.share_path("quad_utils", "launch")

    world_file = param_utils.share_path(
        "gazebo_scripts", "worlds", world, "%s.world" % world)

    actions = []

    # Gazebo world with specific physics parameters
    # (ROS1 gazebo_ros/empty_world.launch -> ROS2 gazebo_ros/gazebo.launch.py)
    actions.append(IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(
            get_package_share_directory("gazebo_ros"),
            "launch", "gazebo.launch.py")),
        launch_arguments={
            "world": world_file,
            "pause": paused,
            "gui": "true" if gui else "false",
            "verbose": "true" if gui else "false",
            "physics": "ode",
        }.items(),
    ))

    # Mapping pipeline
    actions.append(IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(launch_dir, "mapping.launch.py")),
        launch_arguments={
            "input_type": "mesh",
            "world": world,
            "robot_type": robot_type,
            "use_sim_time": "true",
        }.items(),
    ))

    def robot_group(ns, init_pose):
        return GroupAction([
            PushRosNamespace(ns),
            SetParameter("use_sim_time", True),
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource(
                    os.path.join(launch_dir, "quad_spawn.launch.py")),
                launch_arguments={
                    "robot_type": robot_type,
                    "controller": controller,
                    "init_pose": init_pose,
                    "namespace": ns,
                }.items(),
            ),
        ])

    actions.append(robot_group("robot_1", "-x 0.0 -y 0.0 -z 0.5"))

    if multiple_robots:
        actions.append(robot_group("robot_2", "-x 0.0 -y 1.0 -z 0.5"))
        # Users can add more robots here

    # Visualization tools
    actions.append(IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(launch_dir, "quad_visualization.launch.py")),
        launch_arguments={
            "robot_type": robot_type,
            "live_plot": live_plot,
            "dash": dash,
        }.items(),
    ))

    return actions


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument("robot_type", default_value="spirit"),
        DeclareLaunchArgument("multiple_robots", default_value="false"),
        DeclareLaunchArgument("gui", default_value="false"),
        DeclareLaunchArgument("paused", default_value="false"),
        DeclareLaunchArgument("controller", default_value="inverse_dynamics"),
        DeclareLaunchArgument("live_plot", default_value="false"),
        DeclareLaunchArgument("dash", default_value="false"),
        DeclareLaunchArgument("world", default_value="flat"),
        DeclareLaunchArgument("logging", default_value="false"),
        OpaqueFunction(function=launch_setup),
    ])
