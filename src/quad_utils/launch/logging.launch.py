"""ROS2 port of logging.launch (rosbag record -> ros2 bag record)."""

import os
import sys

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, ExecuteProcess, OpaqueFunction
from launch.substitutions import LaunchConfiguration

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import param_utils  # noqa: E402


def launch_setup(context, *args, **kwargs):
    namespace = LaunchConfiguration("namespace").perform(context)
    bag_name = LaunchConfiguration("bag_name").perform(context)

    bags_dir = param_utils.share_path("quad_logger", "bags")
    archive_dir = os.path.join(bags_dir, "archive")
    os.makedirs(archive_dir, exist_ok=True)

    topics = [
        "/%s/state" % namespace,
        "/%s/state/joints" % namespace,
        "/%s/state/imu" % namespace,
        "/%s/state/trajectory" % namespace,
        "/%s/state/ground_truth" % namespace,
        "/%s/state/estimate" % namespace,
        "/%s/state/grfs" % namespace,
        "/%s/mocap_node/quad/pose" % namespace,
        "/%s/global_plan" % namespace,
        "/%s/local_plan" % namespace,
        "/%s/control/grfs" % namespace,
        "/%s/control/joint_command" % namespace,
        "/%s/control/mode" % namespace,
        "/%s/foot_plan_continuous" % namespace,
        "/%s/foot_plan_discrete" % namespace,
        "/terrain_map",
    ]

    # Fixed-name bag (ros2 bag cannot overwrite: remove stale dir first)
    fixed_bag = os.path.join(bags_dir, "%s_%s" % (namespace, bag_name))
    record_fixed = ExecuteProcess(
        cmd=["ros2", "bag", "record", "-o", fixed_bag] + topics,
        output="screen",
    )

    # Timestamped bag in the archive dir (ros2 bag auto-timestamps
    # the output directory when -o is omitted)
    record_archive = ExecuteProcess(
        cmd=["ros2", "bag", "record"] + topics,
        cwd=archive_dir,
        output="screen",
    )

    return [record_fixed, record_archive]


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument("namespace", default_value="robot_1"),
        DeclareLaunchArgument("bag_name", default_value="quad_log"),
        OpaqueFunction(function=launch_setup),
    ])
