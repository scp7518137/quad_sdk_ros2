#include "mocap_optitrack/mocap_config.h"

#include <set>
#include <sstream>

#include <rclcpp/rclcpp.hpp>

namespace mocap_optitrack
{

static const rclcpp::Logger LOGGER = rclcpp::get_logger("mocap_optitrack");

namespace
{
  std::vector<std::string> split(const std::string& s, char delim)
  {
    std::vector<std::string> tokens;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim))
    {
      tokens.push_back(item);
    }
    return tokens;
  }
}

// Server description defaults
const int ServerDescription::Default::CommandPort = 1510;
const int ServerDescription::Default::DataPort   = 9000;
const std::string ServerDescription::Default::MulticastIpAddress = "224.0.0.1";

ServerDescription::ServerDescription() :
  commandPort(ServerDescription::Default::CommandPort),
  dataPort(ServerDescription::Default::DataPort),
  multicastIpAddress(ServerDescription::Default::MulticastIpAddress)
{}

void NodeConfiguration::fromRosParam(
  rclcpp::Node::SharedPtr node,
  ServerDescription& serverDescription,
  PublisherConfigurations& pubConfigs)
{
  // Get server configuration from the parameter server
  std::string multicastAddress;
  if (node->get_parameter("optitrack_config/multicast_address", multicastAddress))
  {
    serverDescription.multicastIpAddress = multicastAddress;
  }
  else
  {
    RCLCPP_WARN(LOGGER, "Could not get multicast address, using default: %s",
      serverDescription.multicastIpAddress.c_str());
  }

  int commandPort = 0;
  if (node->get_parameter("optitrack_config/command_port", commandPort))
  {
    serverDescription.commandPort = commandPort;
  }
  else
  {
    RCLCPP_WARN(LOGGER, "Could not get command port, using default: %d",
      serverDescription.commandPort);
  }

  int dataPort = 0;
  if (node->get_parameter("optitrack_config/data_port", dataPort))
  {
    serverDescription.dataPort = dataPort;
  }
  else
  {
    RCLCPP_WARN(LOGGER, "Could not get data port, using default: %d",
      serverDescription.dataPort);
  }

  std::vector<int64_t> version;
  if (node->get_parameter("optitrack_config/version", version) && version.size() == 4)
  {
    serverDescription.version.assign(version.begin(), version.end());
  }
  else
  {
    RCLCPP_WARN(LOGGER, "Could not get server version, using auto");
  }

  // Parse rigid bodies section
  auto result = node->list_parameters(std::vector<std::string>{"rigid_bodies"}, 10);
  std::set<std::string> bodyIds;
  for (const auto& name : result.names)
  {
    auto parts = split(name, '.');
    if (parts.size() >= 2)
    {
      bodyIds.insert(parts[1]);
    }
  }

  for (const auto& id : bodyIds)
  {
    PublisherConfiguration publisherConfig;
    std::sscanf(id.c_str(), "%d", &publisherConfig.rigidBodyId);

    std::string poseTopicName;
    bool readPoseTopicName = node->get_parameter(
      "rigid_bodies." + id + ".pose", poseTopicName);
    if (!readPoseTopicName)
    {
      RCLCPP_WARN(LOGGER, "Failed to parse pose for body '%s'. Pose publishing disabled.",
        id.c_str());
      publisherConfig.publishPose = false;
    }
    else
    {
      publisherConfig.poseTopicName = poseTopicName;
      publisherConfig.publishPose = true;
    }

    std::string pose2dTopicName;
    bool readPose2dTopicName = node->get_parameter(
      "rigid_bodies." + id + ".pose2d", pose2dTopicName);
    if (!readPose2dTopicName)
    {
      RCLCPP_WARN(LOGGER, "Failed to parse pose2d for body '%s'. Pose2D publishing disabled.",
        id.c_str());
      publisherConfig.publishPose2d = false;
    }
    else
    {
      publisherConfig.pose2dTopicName = pose2dTopicName;
      publisherConfig.publishPose2d = true;
    }

    std::string childFrameId, parentFrameId;
    bool readChildFrameId = node->get_parameter(
      "rigid_bodies." + id + ".child_frame_id", childFrameId);
    bool readParentFrameId = node->get_parameter(
      "rigid_bodies." + id + ".parent_frame_id", parentFrameId);

    if (!readChildFrameId || !readParentFrameId)
    {
      if (!readChildFrameId)
        RCLCPP_WARN(LOGGER, "Failed to parse child_frame_id for body '%s'. TF publishing disabled.",
          id.c_str());
      if (!readParentFrameId)
        RCLCPP_WARN(LOGGER, "Failed to parse parent_frame_id for body '%s'. TF publishing disabled.",
          id.c_str());
      publisherConfig.publishTf = false;
    }
    else
    {
      publisherConfig.childFrameId = childFrameId;
      publisherConfig.parentFrameId = parentFrameId;
      publisherConfig.publishTf = true;
    }

    pubConfigs.push_back(publisherConfig);
  }
}

} // namespace
