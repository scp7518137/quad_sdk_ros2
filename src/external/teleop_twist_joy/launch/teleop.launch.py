"""ROS2 port of teleop_twist_joy teleop.launch."""

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    joy_config = LaunchConfiguration("joy_config")
    joy_dev = LaunchConfiguration("joy_dev")
    joy_topic = LaunchConfiguration("joy_topic")
    config_filepath = LaunchConfiguration("config_filepath")

    default_config = PathJoinSubstitution([
        FindPackageShare("teleop_twist_joy"), "config",
        [joy_config, ".config.yaml"],
    ])

    return LaunchDescription([
        DeclareLaunchArgument("joy_config", default_value="ps3"),
        DeclareLaunchArgument("joy_dev", default_value="/dev/input/js0"),
        DeclareLaunchArgument("config_filepath", default_value=default_config),
        DeclareLaunchArgument("joy_topic", default_value="joy"),

        Node(
            package="joy",
            executable="joy_node",
            name="joy_node",
            parameters=[{
                "dev": joy_dev,
                "deadzone": 0.3,
                "autorepeat_rate": 20.0,
            }],
            remappings=[("joy", joy_topic)],
        ),

        Node(
            package="teleop_twist_joy",
            executable="teleop_node",
            name="teleop_twist_joy",
            output="screen",
            parameters=[config_filepath],
            remappings=[("joy", joy_topic)],
        ),
    ])
