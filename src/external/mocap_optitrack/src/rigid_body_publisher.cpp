#include <mocap_optitrack/rigid_body_publisher.h>

#include <cmath>

namespace mocap_optitrack
{

namespace utilities
{
  geometry_msgs::msg::PoseStamped getRosPose(RigidBody const& body, bool newCoordinates)
  {
    geometry_msgs::msg::PoseStamped poseStampedMsg;
    if (newCoordinates)
    {
      // Motive 1.7+ coordinate system
      poseStampedMsg.pose.position.x = body.pose.position.x;
      poseStampedMsg.pose.position.y = -body.pose.position.z;
      poseStampedMsg.pose.position.z = body.pose.position.y;

      poseStampedMsg.pose.orientation.x = body.pose.orientation.x;
      poseStampedMsg.pose.orientation.y = -body.pose.orientation.z;
      poseStampedMsg.pose.orientation.z = body.pose.orientation.y;
      poseStampedMsg.pose.orientation.w = body.pose.orientation.w;
    }
    else
    {
      // y & z axes are swapped in the Optitrack coordinate system
      poseStampedMsg.pose.position.x = body.pose.position.x;
      poseStampedMsg.pose.position.y = -body.pose.position.z;
      poseStampedMsg.pose.position.z = body.pose.position.y;

      poseStampedMsg.pose.orientation.x = body.pose.orientation.x;
      poseStampedMsg.pose.orientation.y = -body.pose.orientation.z;
      poseStampedMsg.pose.orientation.z = body.pose.orientation.y;
      poseStampedMsg.pose.orientation.w = body.pose.orientation.w;
    }
    return poseStampedMsg;
  }

  double getYaw(geometry_msgs::msg::Quaternion const& q)
  {
    return std::atan2(2.0 * (q.w * q.z + q.x * q.y),
                      1.0 - 2.0 * (q.y * q.y + q.z * q.z));
  }
}

RigidBodyPublisher::RigidBodyPublisher(rclcpp::Node::SharedPtr node,
  Version const& natNetVersion,
  PublisherConfiguration const& config) :
    config(config)
{
  if (config.publishPose)
    posePublisher = node->create_publisher<geometry_msgs::msg::PoseStamped>(
      config.poseTopicName, 1000);

  if (config.publishPose2d)
    pose2dPublisher = node->create_publisher<geometry_msgs::msg::Pose2D>(
      config.pose2dTopicName, 1000);

  if (config.publishTf)
    tfPublisher = std::make_shared<tf2_ros::TransformBroadcaster>(node);

  // Motive 1.7+ uses a new coordinate system
  useNewCoordinates = (natNetVersion >= Version("1.7"));
}

RigidBodyPublisher::~RigidBodyPublisher()
{
}

void RigidBodyPublisher::publish(rclcpp::Time const& time, RigidBody const& body)
{
  // don't do anything if no new data was provided
  if (!body.hasValidData())
  {
    return;
  }

  // NaN?
  if (body.pose.position.x != body.pose.position.x)
  {
    return;
  }

  geometry_msgs::msg::PoseStamped pose = utilities::getRosPose(body, useNewCoordinates);
  pose.header.stamp = time;

  if (config.publishPose)
  {
    pose.header.frame_id = config.parentFrameId;
    posePublisher->publish(pose);
  }

  // publish 2D pose
  if (config.publishPose2d)
  {
    geometry_msgs::msg::Pose2D pose2d;
    pose2d.x = pose.pose.position.x;
    pose2d.y = pose.pose.position.y;
    pose2d.theta = utilities::getYaw(pose.pose.orientation);
    pose2dPublisher->publish(pose2d);
  }

  if (config.publishTf && tfPublisher)
  {
    // publish transform
    geometry_msgs::msg::TransformStamped transform;
    transform.header.stamp = time;
    transform.header.frame_id = config.parentFrameId;
    transform.child_frame_id = config.childFrameId;

    transform.transform.translation.x = pose.pose.position.x;
    transform.transform.translation.y = pose.pose.position.y;
    transform.transform.translation.z = pose.pose.position.z;
    transform.transform.rotation = pose.pose.orientation;

    tfPublisher->sendTransform(transform);
  }
}


RigidBodyPublishDispatcher::RigidBodyPublishDispatcher(
  rclcpp::Node::SharedPtr node,
  Version const& natNetVersion,
  PublisherConfigurations const& configs)
{
  for (auto const& config : configs)
  {
    rigidBodyPublisherMap[config.rigidBodyId] =
      RigidBodyPublisherPtr(new RigidBodyPublisher(node, natNetVersion, config));
  }
}

void RigidBodyPublishDispatcher::publish(
  rclcpp::Time const& time,
  std::vector<RigidBody> const& rigidBodies)
{
  for (auto const& rigidBody : rigidBodies)
  {
    auto const& iter = rigidBodyPublisherMap.find(rigidBody.bodyId);

    if (iter != rigidBodyPublisherMap.end())
    {
      (*iter->second).publish(time, rigidBody);
    }
  }
}


} // namespace
