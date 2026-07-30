"""ROS2 port of grid_map_pcl_loader_node.launch."""

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    default_folder = PathJoinSubstitution(
        [FindPackageShare("grid_map_pcl"), "data"])
    parameters_yaml = PathJoinSubstitution(
        [FindPackageShare("grid_map_pcl"), "config", "parameters.yaml"])

    return LaunchDescription([
        DeclareLaunchArgument("folder_path", default_value=default_folder),
        DeclareLaunchArgument("pcd_filename", default_value="plane_noisy.pcd"),
        DeclareLaunchArgument("map_rosbag_topic", default_value="grid_map"),
        DeclareLaunchArgument("output_grid_map",
                              default_value="elevation_map.bag"),
        DeclareLaunchArgument("map_frame", default_value="map"),
        DeclareLaunchArgument("map_layer_name", default_value="elevation"),
        DeclareLaunchArgument("prefix", default_value=""),
        DeclareLaunchArgument("set_verbosity_to_debug", default_value="false"),

        Node(
            package="grid_map_pcl",
            executable="grid_map_pcl_loader_node",
            name="grid_map_pcl_loader_node",
            output="screen",
            prefix=LaunchConfiguration("prefix"),
            parameters=[
                parameters_yaml,
                {
                    "folder_path": LaunchConfiguration("folder_path"),
                    "pcd_filename": LaunchConfiguration("pcd_filename"),
                    "map_rosbag_topic": LaunchConfiguration("map_rosbag_topic"),
                    "output_grid_map": LaunchConfiguration("output_grid_map"),
                    "map_frame": LaunchConfiguration("map_frame"),
                    "map_layer_name": LaunchConfiguration("map_layer_name"),
                    "set_verbosity_to_debug":
                        LaunchConfiguration("set_verbosity_to_debug"),
                },
            ],
        ),
    ])
