"""Standalone terrain map viewer from image — slope.png or ground.png, no planner.

Usage:
  ros2 launch global_body_planner terrain_viewer.launch.py map:=slope
  ros2 launch global_body_planner terrain_viewer.launch.py map:=ground
"""

import os

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

from ament_index_python.packages import get_package_share_directory


def launch_setup(context, *args, **kwargs):
    map_name = LaunchConfiguration("map").perform(context)
    quad_utils_share = get_package_share_directory("quad_utils")
    image_path = os.path.join(quad_utils_share, "data", map_name + ".png")

    actions = [
        # TF: map frame at origin
        Node(
            package="tf2_ros",
            executable="static_transform_publisher",
            name="map_tf",
            arguments=["0", "0", "0", "0", "0", "0", "map", "world"],
        ),
        # Image publisher: read PNG -> publish sensor_msgs/Image
        # NOTE: image_publisher_node takes the file path as a positional ARGUMENT, not a param
        Node(
            package="image_publisher",
            executable="image_publisher_node",
            name="image_publisher",
            output="screen",
            arguments=[image_path],
            parameters=[{"publish_rate": 5.0}],
            remappings=[("image_raw", "/image_publisher/image")],
        ),
        # Terrain map publisher: image -> GridMap
        Node(
            package="quad_utils",
            executable="terrain_map_publisher_node",
            name="terrain_map_publisher",
            output="screen",
            parameters=[{
                "terrain_map_publisher": {
                    "map_data_source": "image",
                    "resolution": 0.2,
                    "min_height": 0.0,
                    "max_height": 1.0,
                },
                "topics": {
                    "terrain_map_raw": "/terrain_map",
                },
            }],
        ),
    ]

    # RViz
    rviz_config = os.path.join(
        get_package_share_directory("quad_utils"),
        "rviz", "quad_viewer.rviz")
    rviz_args = ["-d", rviz_config] if os.path.exists(rviz_config) else []
    actions.append(Node(
        package="rviz2",
        executable="rviz2",
        name="rviz",
        arguments=rviz_args,
    ))

    return actions


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument("map", default_value="slope",
                              description="Map image name: slope or ground"),
        OpaqueFunction(function=launch_setup),
    ])
