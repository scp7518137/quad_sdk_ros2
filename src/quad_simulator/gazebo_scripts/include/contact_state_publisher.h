#ifndef CONTACT_STATE_PUBLISHER_H
#define CONTACT_STATE_PUBLISHER_H

#include <gazebo_msgs/msg/contacts_state.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <quad_msgs/msg/grf_array.hpp>
#include <quad_utils/ros_utils.h>
#include <rclcpp/rclcpp.hpp>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>

#include <cmath>
#include <memory>
#include <string>

class ContactStatePublisher : public rclcpp::Node {
 public:
  ContactStatePublisher();

  /// Must be called after make_shared to safely use shared_from_this().
  void init();

 private:
  void contactStateCallback(
      const gazebo_msgs::msg::ContactsState& msg, const int toe_idx);
  void publishContactState();

  rclcpp::Subscription<gazebo_msgs::msg::ContactsState>::SharedPtr
      toe0_contact_state_sub;
  rclcpp::Subscription<gazebo_msgs::msg::ContactsState>::SharedPtr
      toe1_contact_state_sub;
  rclcpp::Subscription<gazebo_msgs::msg::ContactsState>::SharedPtr
      toe2_contact_state_sub;
  rclcpp::Subscription<gazebo_msgs::msg::ContactsState>::SharedPtr
      toe3_contact_state_sub;

  std::unique_ptr<tf2_ros::Buffer> buffer_;
  std::shared_ptr<tf2_ros::TransformListener> listener_;
  rclcpp::Publisher<quad_msgs::msg::GRFArray>::SharedPtr grf_pub_;
  rclcpp::TimerBase::SharedPtr timer_;

  double update_rate_;
  const int num_feet_ = 4;
  quad_msgs::msg::GRFArray grf_array_msg_;
  bool ready_to_publish_;
};

#endif
