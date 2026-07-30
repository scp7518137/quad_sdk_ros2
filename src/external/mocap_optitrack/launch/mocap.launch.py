"""ROS2 port of mocap_optitrack mocap.launch."""

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, Shutdown
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    default_config = PathJoinSubstitution(
        [FindPackageShare("mocap_optitrack"), "config", "mocap.yaml"])

    return LaunchDescription([
        # Pass mocap_config_file:=/path/to/config.yaml to change options.
        DeclareLaunchArgument("mocap_config_file",
                              default_value=default_config),
        Node(
            package="mocap_optitrack",
            executable="mocap_node",
            name="mocap_node",
            respawn=False,
            parameters=[LaunchConfiguration("mocap_config_file")],
            # ROS1 required="true" -> shut everything down if it exits
            on_exit=Shutdown(),
        ),
    ])
