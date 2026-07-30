"""Shared helpers for quad-sdk ROS2 launch files.

In ROS1 the *.launch files loaded YAML files onto a global parameter server
(load_global_params.launch / load_robot_params.launch). ROS2 has no global
parameter server: every node must be given its parameter files explicitly.

This module centralizes the lists of parameter files so that each launch file
can attach them to the nodes it starts. All YAML files have been converted to
the ROS2 format (wildcard "/**:" + "ros__parameters:").
"""

import os

from ament_index_python.packages import get_package_share_directory


def share_path(package, *path):
    """Return an absolute path inside a package's share directory."""
    return os.path.join(get_package_share_directory(package), *path)


def str2bool(value):
    """Convert a launch argument string to bool."""
    return str(value).lower() in ("true", "1", "yes")


def global_param_files():
    """Parameter files formerly loaded by load_global_params.launch."""
    return [
        share_path("global_body_planner", "global_body_planner.yaml"),
        share_path("local_planner", "local_planner.yaml"),
        share_path("nmpc_controller", "nmpc_controller.yaml"),
        share_path("robot_driver", "robot_driver.yaml"),
        share_path("quad_utils", "config", "terrain_map_publisher.yaml"),
        share_path("quad_utils", "config", "remote_heartbeat.yaml"),
        share_path("quad_utils", "config", "teleop_twist_keyboard.yaml"),
        share_path("quad_utils", "config", "rviz_interface.yaml"),
        share_path("quad_utils", "config", "trajectory_publisher.yaml"),
        share_path("quad_utils", "config", "topics_global.yaml"),
    ]


def robot_param_files(robot_type):
    """Parameter files formerly loaded by load_robot_params.launch."""
    return [
        share_path("quad_utils", "config", "topics_robot.yaml"),
        share_path("quad_utils", "config", "%s.yaml" % robot_type),
    ]


def robot_description(robot_type):
    """Return the URDF contents for the given robot type (spirit / a1)."""
    urdf_path = share_path(
        "%s_description" % robot_type, "urdf", "%s.urdf" % robot_type)
    with open(urdf_path, "r") as f:
        return f.read()


def robot_sdf_path(robot_type):
    """Return the path of the SDF model for the given robot type."""
    return share_path(
        "%s_description" % robot_type, "sdf_mesh", "%s.sdf" % robot_type)


def robot_params(robot_type, use_sim_time=False, with_description=True):
    """Parameter list (files + dict) to attach to quad-sdk nodes.

    Equivalent of including load_robot_params.launch in ROS1.
    """
    params = list(robot_param_files(robot_type))
    extra = {
        "enable_statistics": True,
        "robot_type": robot_type,
        "use_sim_time": use_sim_time,
    }
    if with_description:
        extra["robot_description"] = robot_description(robot_type)
    params.append(extra)
    return params


def global_params(robot_type, use_sim_time=False, with_description=True):
    """Equivalent of including load_global_params.launch in ROS1."""
    return global_param_files() + robot_params(
        robot_type, use_sim_time=use_sim_time, with_description=with_description)
