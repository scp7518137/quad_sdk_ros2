"""ROS2 port of planning.launch."""

import ast
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
    reference = LaunchConfiguration("reference").perform(context)
    logging = param_utils.str2bool(
        LaunchConfiguration("logging").perform(context))
    twist_input = LaunchConfiguration("twist_input").perform(context)
    namespace = LaunchConfiguration("namespace").perform(context)
    robot_type = LaunchConfiguration("robot_type").perform(context)
    leaping = param_utils.str2bool(
        LaunchConfiguration("leaping").perform(context))
    ac = param_utils.str2bool(LaunchConfiguration("ac").perform(context))
    goal_state = LaunchConfiguration("goal_state").perform(context)
    use_sim_time = param_utils.str2bool(
        LaunchConfiguration("use_sim_time").perform(context))

    launch_dir = param_utils.share_path("quad_utils", "launch")
    global_params = param_utils.global_params(
        robot_type, use_sim_time=use_sim_time)
    use_twist_input = (reference == "twist")

    actions = []

    # Global body planner (reference == 'gbpl')
    if reference == "gbpl":
        gbp_params = global_params + [{
            "enable_leaping": leaping,
            "local_planner.use_twist_input": False,
        }]
        if goal_state:
            gbp_params.append({
                "global_body_planner.goal_state":
                    [float(v) for v in ast.literal_eval(goal_state)],
            })
        actions.append(Node(
            package="global_body_planner",
            executable="global_body_planner_node",
            name="global_body_planner",
            output="screen",
            remappings=[
                ("start_state", "state/ground_truth"),
                ("goal_state", "clicked_point"),
            ],
            parameters=gbp_params,
        ))

    # Twist input to local plan, no global plan (reference == 'twist')
    if use_twist_input:
        if twist_input == "keyboard":
            actions.append(Node(
                package="teleop_twist_keyboard",
                executable="teleop_twist_keyboard",
                name="teleop_twist_keyboard",
                prefix="xterm -e",
                parameters=[{"use_sim_time": use_sim_time}],
            ))
        if twist_input == "joy":
            actions.append(IncludeLaunchDescription(
                PythonLaunchDescriptionSource(
                    param_utils.share_path(
                        "teleop_twist_joy", "launch", "teleop.launch.py")),
                launch_arguments={"joy_config": "ps3-holonomic"}.items(),
            ))

    # Local planner
    actions.append(Node(
        package="local_planner",
        executable="local_planner_node",
        name="local_planner",
        output="screen",
        parameters=global_params + [{
            "local_planner.use_twist_input": use_twist_input,
            "nmpc_controller.enable_adaptive_complexity": ac,
        }],
    ))

    if logging:
        actions.append(IncludeLaunchDescription(
            PythonLaunchDescriptionSource(
                os.path.join(launch_dir, "logging.launch.py")),
            launch_arguments={"namespace": namespace}.items(),
        ))

    # Plan publisher
    actions.append(Node(
        package="quad_utils",
        executable="trajectory_publisher_node",
        name="plan_publisher",
        output="screen",
        parameters=global_params,
    ))

    return actions


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument("reference", default_value="twist"),
        DeclareLaunchArgument("logging", default_value="false"),
        DeclareLaunchArgument("twist_input", default_value="none"),
        DeclareLaunchArgument("namespace", default_value="robot_1"),
        DeclareLaunchArgument("robot_type", default_value="spirit"),
        DeclareLaunchArgument("leaping", default_value="true"),
        DeclareLaunchArgument("ac", default_value="false"),
        # Optional override of global_body_planner goal state, e.g. "[5.0, 1.0]"
        DeclareLaunchArgument("goal_state", default_value=""),
        DeclareLaunchArgument("use_sim_time", default_value="false"),
        OpaqueFunction(function=launch_setup),
    ])
