#include <memory>
#include <unistd.h>

#include <mocap_optitrack/socket.h>
#include <mocap_optitrack/data_model.h>
#include <mocap_optitrack/mocap_config.h>
#include <mocap_optitrack/rigid_body_publisher.h>
#include "natnet/natnet_messages.h"

#include <rclcpp/rclcpp.hpp>


namespace mocap_optitrack
{

  static const rclcpp::Logger LOGGER = rclcpp::get_logger("mocap_optitrack");

  class OptiTrackRosBridge
  {
  public:
    OptiTrackRosBridge(rclcpp::Node::SharedPtr node,
      ServerDescription const& serverDescr,
      PublisherConfigurations const& pubConfigs) :
        node_(node),
        serverDescription(serverDescr),
        publisherConfigurations(pubConfigs)
    {

    }

    void initialize()
    {
      // Create socket
      multicastClientSocketPtr.reset(
        new UdpMulticastSocket(serverDescription.dataPort,
          serverDescription.multicastIpAddress));

      if (!serverDescription.version.empty())
      {
        dataModel.setVersions(&serverDescription.version[0], &serverDescription.version[0]);
      }

      // Need version information from the server to properly decode any of their packets.
      // If we have not received that yet, send another request.
      while(rclcpp::ok() && !dataModel.hasServerInfo())
      {
        natnet::ConnectionRequestMessage connectionRequestMsg;
        natnet::MessageBuffer connectionRequestMsgBuffer;
        connectionRequestMsg.serialize(connectionRequestMsgBuffer, NULL);
        int ret = multicastClientSocketPtr->send(
          &connectionRequestMsgBuffer[0],
          connectionRequestMsgBuffer.size(),
          serverDescription.commandPort);

        if (updateDataModelFromServer()) usleep(10);
      }

      // Once we have the server info, create publishers
      publishDispatcherPtr.reset(
        new RigidBodyPublishDispatcher(node_,
          dataModel.getNatNetVersion(),
          publisherConfigurations));

      RCLCPP_INFO(LOGGER, "Initialization complete");
    };

    void run()
    {
      rclcpp::Time t_now = node_->now();
      while (rclcpp::ok())
      {
        if (updateDataModelFromServer())
        {
          // Maybe we got some data? If we did it would be in the form of one or more
          // rigid bodies in the data model
          rclcpp::Time time = node_->now();
          publishDispatcherPtr->publish(time, dataModel.dataFrame.rigidBodies);
          t_now = time;

          // Clear out the model to prepare for the next frame of data
          dataModel.clear();

          // If we processed some data, take a short break
          usleep( 10 );
        }
      }
    }

  private:
    bool updateDataModelFromServer()
    {
      // Get data from mocap server
      int numBytesReceived = multicastClientSocketPtr->recv();
      if( numBytesReceived > 0 )
      {
        // Grab latest message buffer
        const char* pMsgBuffer = multicastClientSocketPtr->getBuffer();

        // Copy char* buffer into MessageBuffer and dispatch to be deserialized
        natnet::MessageBuffer msgBuffer(pMsgBuffer, pMsgBuffer + numBytesReceived);
        natnet::MessageDispatcher::dispatch(msgBuffer, &dataModel);

        return true;
      }

      return false;
    };

    rclcpp::Node::SharedPtr node_;
    ServerDescription serverDescription;
    PublisherConfigurations publisherConfigurations;
    DataModel dataModel;
    std::unique_ptr<UdpMulticastSocket> multicastClientSocketPtr;
    std::unique_ptr<RigidBodyPublishDispatcher> publishDispatcherPtr;
  };

} // namespace


////////////////////////////////////////////////////////////////////////
int main( int argc, char* argv[] )
{
  // Initialize ROS node
  rclcpp::init(argc, argv);
  auto node = rclcpp::Node::make_shared("mocap_node",
    rclcpp::NodeOptions()
      .allow_undeclared_parameters(true)
      .automatically_declare_parameters_from_overrides(true));

  // Grab node configuration from parameters
  mocap_optitrack::ServerDescription serverDescription;
  mocap_optitrack::PublisherConfigurations publisherConfigurations;
  mocap_optitrack::NodeConfiguration::fromRosParam(node, serverDescription, publisherConfigurations);

  // Create node object, initialize and run
  mocap_optitrack::OptiTrackRosBridge bridge(node, serverDescription, publisherConfigurations);
  bridge.initialize();
  bridge.run();

  rclcpp::shutdown();
  return 0;
}
