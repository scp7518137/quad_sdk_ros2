"""ROS2 port of global_body_planner diagnostics.launch."""

import os
import sys

from launch import LaunchDescription
from launch.actions import OpaqueFunction
from launch_ros.actions import Node

from ament_index_python.packages import get_package_share_directory

sys.path.insert(0, os.path.join(
    get_package_share_directory("quad_utils"), "launch"))
import param_utils  # noqa: E402


def launch_setup(context, *args, **kwargs):
    global_params = param_utils.global_params("spirit")

    # Diagnostics config (kept from ROS1; attach only if present)
    diagnostics_yaml = os.path.join(
        get_package_share_directory("global_body_planner"),
        "config", "diagnostics.yaml")
    extra_params = [diagnostics_yaml] if os.path.exists(diagnostics_yaml) else []

    actions = [
        # Generate simple terrain
        Node(
            package="quad_utils",
            executable="terrain_map_publisher_node",
            name="terrain_map_publisher",
            output="screen",
            parameters=global_params + extra_params,
        ),
        # Load the image of the terrain map
        Node(
            package="grid_map_demos",
            executable="image_publisher.py",
            name="image_publisher",
            output="screen",
            parameters=[{
                "image_path": param_utils.share_path(
                    "quad_utils", "data", "slope.png"),
                "topic": "~image",
            }],
        ),
        # Global body planner
        Node(
            package="global_body_planner",
            executable="global_body_planner_node",
            name="global_body_planner",
            output="screen",
            parameters=global_params + extra_params,
        ),
        # Grid map visualizer
        Node(
            package="grid_map_visualization",
            executable="grid_map_visualization",
            name="grid_map_visualization",
            output="screen",
            parameters=global_params,
        ),
        # Publish the visual topics for the plans
        Node(
            package="quad_utils",
            executable="rviz_interface_node",
            name="rviz_interface",
            output="screen",
            parameters=global_params,
        ),
    ]

    # RViz (config kept from ROS1; only pass -d if the file exists)
    rviz_config = os.path.join(
        get_package_share_directory("global_body_planner"),
        "rviz", "example_with_planner_config.terrain.rviz")
    rviz_args = ["-d", rviz_config] if os.path.exists(rviz_config) else []
    actions.append(Node(
        package="rviz2",
        executable="rviz2",
        name="rviz",
        arguments=rviz_args,
    ))

    return actions


def generate_launch_description():
    return LaunchDescription([OpaqueFunction(function=launch_setup)])
