"""ROS2 port of quad_spawn.launch."""

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
    controller = LaunchConfiguration("controller").perform(context)
    init_pose = LaunchConfiguration("init_pose").perform(context)
    namespace = LaunchConfiguration("namespace").perform(context)
    robot_type = LaunchConfiguration("robot_type").perform(context)

    launch_dir = param_utils.share_path("quad_utils", "launch")
    sdf_path = param_utils.robot_sdf_path(robot_type)

    # Spawn SDF model (gazebo_ros spawn_model -> spawn_entity.py)
    spawn_args = ["-entity", namespace, "-file", sdf_path,
                  "-robot_namespace", namespace]
    spawn_args += init_pose.split()
    spawn_sdf_model = Node(
        package="gazebo_ros",
        executable="spawn_entity.py",
        name="spawn_sdf_model",
        arguments=spawn_args,
        output="screen",
    )

    # Start Quad-SDK robot driver
    robot_driver = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(launch_dir, "robot_driver.launch.py")),
        launch_arguments={
            "robot_type": robot_type,
            "controller": controller,
            "mocap": "false",
            "is_hardware": "false",
            "use_sim_time": "true",
        }.items(),
    )

    # Start gazebo controller plugins
    # (ROS1 controller_manager/spawner -> ros2_control spawner)
    quad_control_yaml = param_utils.share_path(
        "gazebo_scripts", "config", "quad_control.yaml")
    controller_spawner = Node(
        package="controller_manager",
        executable="spawner",
        name="controller_spawner",
        respawn=False,
        output="screen",
        arguments=["joint_controller", "joint_state_controller",
                   "--param-file", quad_control_yaml],
    )

    # Load the contact state publisher
    contact_state_publisher = Node(
        package="gazebo_scripts",
        executable="contact_state_publisher_node",
        name="contact_state_publisher",
        output="screen",
        parameters=param_utils.global_params(robot_type, use_sim_time=True),
    )

    return [spawn_sdf_model, robot_driver, controller_spawner,
            contact_state_publisher]


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument("controller", default_value="inverse_dynamics"),
        DeclareLaunchArgument("init_pose", default_value="-x 0.0 -y 0.0 -z 0.5"),
        DeclareLaunchArgument("namespace", default_value="robot_1"),
        DeclareLaunchArgument("robot_type", default_value="spirit"),
        OpaqueFunction(function=launch_setup),
    ])
