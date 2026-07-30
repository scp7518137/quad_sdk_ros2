#ifndef REMOTE_HEARTBEAT_H
#define REMOTE_HEARTBEAT_H

#include <quad_msgs/msg/leg_command.hpp>
#include <quad_utils/ros_utils.h>
#include <rclcpp/rclcpp.hpp>

//! A class for implementing a remote heartbeat
/*!
   RemoteHeartbeat publishes stamped messages at a fixed rate as a heartbeat
*/
class RemoteHeartbeat {
 public:
  /**
   * @brief Constructor for RemoteHeartbeat Class
   * @param[in] node ROS Node to publish and subscribe from
   * @return Constructed object of type RemoteHeartbeat
   */
  RemoteHeartbeat(rclcpp::Node::SharedPtr node);

  /**
   * @brief Calls ros spinOnce and pubs data at set frequency
   */
  void spin();

 private:
  /**
   * @brief Callback function to handle new robot heartbeat
   * @param[in] msg header containing robot heartbeat
   */
  void robotHeartbeatCallback(const std_msgs::msg::Header::SharedPtr msg);

  /// Nodehandle to pub to and sub from
  rclcpp::Node::SharedPtr node_;

  /// Subscriber for robot heartbeat messages
  rclcpp::Subscription<std_msgs::msg::Header>::SharedPtr robot_heartbeat_sub_;

  /// ROS publisher for remote heartbeat messages
  rclcpp::Publisher<std_msgs::msg::Header>::SharedPtr remote_heartbeat_pub_;

  /// Update rate for sending and receiving data
  double update_rate_;
   
  /// Latency threshold on robot messages for warnings (s)
  double robot_latency_threshold_warn_;

  /// Latency threshold on robot messages for error (s)
  double robot_latency_threshold_error_;
};

#endif  // REMOTE_HEARTBEAT_H
