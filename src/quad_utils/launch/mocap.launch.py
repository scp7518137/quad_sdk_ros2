"""ROS2 port of quad_utils mocap.launch."""

from launch import LaunchDescription
from launch.actions import Shutdown
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare
from launch.substitutions import PathJoinSubstitution


def generate_launch_description():
    mocap_config = PathJoinSubstitution(
        [FindPackageShare("mocap_optitrack"), "config", "mocap.yaml"])

    return LaunchDescription([
        Node(
            package="mocap_optitrack",
            executable="mocap_node",
            name="mocap_node",
            respawn=False,
            output="screen",
            parameters=[mocap_config],
            # ROS1 required="true" -> shut everything down if it exits
            on_exit=Shutdown(),
        ),
    ])
