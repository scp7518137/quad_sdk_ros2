"""ROS2 port of mapping.launch."""

import os
import sys

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import param_utils  # noqa: E402


def launch_setup(context, *args, **kwargs):
    robot_type = LaunchConfiguration("robot_type").perform(context)
    input_type = LaunchConfiguration("input_type").perform(context)
    use_sim_time = param_utils.str2bool(
        LaunchConfiguration("use_sim_time").perform(context))

    global_params = param_utils.global_params(
        robot_type, use_sim_time=use_sim_time)

    actions = []

    if input_type == "grid":
        # Generate simple terrain from csv or compute in node
        actions.append(Node(
            package="quad_utils",
            executable="terrain_map_publisher_node",
            name="terrain_map_publisher",
            output="screen",
            parameters=global_params,
        ))

    if input_type == "mesh":
        # Generate the grid map from a mesh
        actions.append(Node(
            package="quad_utils",
            executable="mesh_to_grid_map_node",
            name="mesh_to_grid_map_node",
            output="screen",
            parameters=global_params + [{
                "frame_id_mesh_loaded":
                    LaunchConfiguration("frame_id_mesh_loaded").perform(context),
                "grid_map_resolution": float(
                    LaunchConfiguration("grid_map_resolution").perform(context)),
                "layer_name":
                    LaunchConfiguration("grid_map_layer_name").perform(context),
                "latch_grid_map_pub": param_utils.str2bool(
                    LaunchConfiguration("latch_grid_map_pub").perform(context)),
                "verbose": param_utils.str2bool(
                    LaunchConfiguration("verbose").perform(context)),
                "world": LaunchConfiguration("world").perform(context),
            }],
        ))

    # Grid map visualizer
    actions.append(Node(
        package="grid_map_visualization",
        executable="grid_map_visualization",
        name="grid_map_visualization",
        output="screen",
        parameters=global_params + [param_utils.share_path(
            "quad_utils", "config", "grid_map_visualization.yaml")],
    ))

    # Grid map filters demo node
    actions.append(Node(
        package="grid_map_demos",
        executable="filters_demo",
        name="grid_map_filter_node",
        parameters=[
            param_utils.share_path("quad_utils", "config", "filter_chain.yaml"),
            {
                "input_topic": "/terrain_map_raw",
                "output_topic": "/terrain_map",
                "use_sim_time": use_sim_time,
            },
        ],
    ))

    return actions


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument("robot_type", default_value="spirit"),
        # Type of input (grid, mesh, cloud)
        DeclareLaunchArgument("input_type", default_value="grid"),
        # Args for mesh loader
        DeclareLaunchArgument("frame_id_mesh_loaded", default_value="map"),
        DeclareLaunchArgument("grid_map_layer_name", default_value="z"),
        DeclareLaunchArgument("grid_map_resolution", default_value="0.05"),
        DeclareLaunchArgument("latch_grid_map_pub", default_value="true"),
        DeclareLaunchArgument("verbose", default_value="true"),
        DeclareLaunchArgument("world", default_value="step_20cm"),
        DeclareLaunchArgument("use_sim_time", default_value="false"),
        OpaqueFunction(function=launch_setup),
    ])
