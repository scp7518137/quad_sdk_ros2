#ifndef __MOCAP_OPTITRACK_MOCAP_CONFIG_H__
#define __MOCAP_OPTITRACK_MOCAP_CONFIG_H__

#include <vector>
#include <string>

#include <memory>
#include <rclcpp/rclcpp.hpp>

namespace mocap_optitrack
{

/// \brief Server communication info
struct ServerDescription
{
  struct Default
  {
    static const int CommandPort;
    static const int DataPort;
    static const std::string MulticastIpAddress;
  };

  ServerDescription();
  int commandPort;
  int dataPort;
  std::string multicastIpAddress;
  std::vector<int> version;
};

/// \brief ROS publisher configuration
struct PublisherConfiguration
{
  int rigidBodyId;
  std::string poseTopicName;
  std::string pose2dTopicName;
  std::string childFrameId;
  std::string parentFrameId;

  bool publishPose;
  bool publishPose2d;
  bool publishTf;
};

typedef std::vector<PublisherConfiguration> PublisherConfigurations;

/// \brief Handles loading node configuration from different sources
struct NodeConfiguration
{
  static void fromRosParam(rclcpp::Node::SharedPtr node,
    ServerDescription& serverDescription,
    PublisherConfigurations& pubConfigs);
};

} // namespace

#endif  // __MOCAP_OPTITRACK_MOCAP_CONFIG_H__
