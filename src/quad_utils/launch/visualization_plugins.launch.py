"""ROS2 port of visualization_plugins.launch."""

import os
import sys

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import param_utils  # noqa: E402


def launch_setup(context, *args, **kwargs):
    namespace = LaunchConfiguration("namespace").perform(context)
    robot_type = LaunchConfiguration("robot_type").perform(context)

    description = param_utils.robot_description(robot_type)

    def state_publisher(sub_ns):
        # ROS1 tf_prefix -> ROS2 frame_prefix
        return Node(
            package="robot_state_publisher",
            executable="robot_state_publisher",
            name="robot_state_publisher",
            namespace=sub_ns,
            remappings=[
                ("joint_states", "visualization/joint_states"),
                ("robot_description", "/robot_description"),
            ],
            parameters=[{
                "robot_description": description,
                "frame_prefix": "%s_%s/" % (namespace, sub_ns),
            }],
        )

    rviz_interface = Node(
        package="quad_utils",
        executable="rviz_interface_node",
        name="rviz_interface",
        output="screen",
        parameters=param_utils.global_params(robot_type),
    )

    return [
        state_publisher("trajectory"),
        state_publisher("ground_truth"),
        rviz_interface,
    ]


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument("namespace", default_value="robot_1"),
        DeclareLaunchArgument("robot_type", default_value="spirit"),
        OpaqueFunction(function=launch_setup),
    ])
