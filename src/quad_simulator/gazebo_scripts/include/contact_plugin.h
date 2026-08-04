#ifndef _GAZEBO_CONTACT_PLUGIN_HH_
#define _GAZEBO_CONTACT_PLUGIN_HH_

#include <geometry_msgs/msg/vector3.hpp>
#include <quad_msgs/msg/contact_mode.hpp>
#include <rclcpp/rclcpp.hpp>

#include <gazebo/gazebo.hh>
#include <gazebo/sensors/sensors.hh>
#include <memory>
#include <string>

namespace gazebo {

/// \brief A contact sensor plugin that publishes contact forces to ROS2.
class ContactPlugin : public SensorPlugin {
 public:
  ContactPlugin();
  virtual ~ContactPlugin();

  /// \brief Load the sensor plugin.
  virtual void Load(sensors::SensorPtr _sensor, sdf::ElementPtr _sdf);

 private:
  /// \brief Callback that receives the contact sensor's update signal.
  virtual void OnUpdate();

  /// \brief Pointer to the contact sensor
  sensors::ContactSensorPtr parentSensor;

  /// \brief Connection that maintains a link between the contact sensor's
  /// updated signal and the OnUpdate callback.
  event::ConnectionPtr updateConnection;

  /// \brief ROS2 node for publishing
  rclcpp::Node::SharedPtr ros_node_;

  /// \brief ROS2 publisher for contact forces
  rclcpp::Publisher<quad_msgs::msg::ContactMode>::SharedPtr contact_publisher_;
};

}  // namespace gazebo
#endif
