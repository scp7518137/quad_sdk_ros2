#ifndef __MOCAP_OPTITRACK_RIGID_BODY_PUBLISHER_H__
#define __MOCAP_OPTITRACK_RIGID_BODY_PUBLISHER_H__

#include <map>
#include <memory>

#include <rclcpp/rclcpp.hpp>
#include <tf2_ros/transform_broadcaster.h>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <geometry_msgs/msg/pose2_d.hpp>

#include <mocap_optitrack/version.h>
#include <mocap_optitrack/data_model.h>
#include <mocap_optitrack/mocap_config.h>

namespace mocap_optitrack
{

/// \brief Encapsulation of a RigidBody data publisher.
class RigidBodyPublisher
{
public:
  RigidBodyPublisher(rclcpp::Node::SharedPtr node,
    Version const& natNetVersion,
    PublisherConfiguration const& config);
  ~RigidBodyPublisher();
  void publish(rclcpp::Time const& time, RigidBody const&);

private:
  PublisherConfiguration config;

  bool useNewCoordinates;

  std::shared_ptr<tf2_ros::TransformBroadcaster> tfPublisher;
  rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr posePublisher;
  rclcpp::Publisher<geometry_msgs::msg::Pose2D>::SharedPtr pose2dPublisher;
};

/// \brief Dispatches RigidBody data to the correct publisher.
class RigidBodyPublishDispatcher
{
    typedef std::shared_ptr<RigidBodyPublisher> RigidBodyPublisherPtr;
    typedef std::map<int,RigidBodyPublisherPtr> RigidBodyPublisherMap;
    RigidBodyPublisherMap rigidBodyPublisherMap;

public:
    RigidBodyPublishDispatcher(rclcpp::Node::SharedPtr node,
        Version const& natNetVersion,
        PublisherConfigurations const& configs);
    void publish(rclcpp::Time const& time, std::vector<RigidBody> const&);
};

} // namespace

#endif
