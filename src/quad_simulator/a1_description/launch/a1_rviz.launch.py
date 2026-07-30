"""ROS2 port of a1_rviz.launch."""

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import (Command, LaunchConfiguration,
                                  PathJoinSubstitution)
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    user_debug = LaunchConfiguration("user_debug")

    xacro_file = PathJoinSubstitution(
        [FindPackageShare("a1_description"), "xacro", "robot.xacro"])
    rviz_config = PathJoinSubstitution(
        [FindPackageShare("a1_description"), "launch", "check_joint.rviz"])

    robot_description = ParameterValue(
        Command(["xacro ", xacro_file, " DEBUG:=", user_debug]),
        value_type=str)

    return LaunchDescription([
        DeclareLaunchArgument("user_debug", default_value="false"),

        # Send fake joint values (ROS1 use_gui -> joint_state_publisher_gui)
        Node(
            package="joint_state_publisher_gui",
            executable="joint_state_publisher_gui",
            name="joint_state_publisher",
        ),

        Node(
            package="robot_state_publisher",
            executable="robot_state_publisher",
            name="robot_state_publisher",
            parameters=[{
                "robot_description": robot_description,
                "publish_frequency": 1000.0,
            }],
        ),

        Node(
            package="rviz2",
            executable="rviz2",
            name="rviz",
            respawn=False,
            output="screen",
            arguments=["-d", rviz_config],
        ),
    ])
