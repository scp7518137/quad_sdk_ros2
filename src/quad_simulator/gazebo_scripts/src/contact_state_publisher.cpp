#include "contact_state_publisher.h"

#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

ContactStatePublisher::ContactStatePublisher()
    : Node("contact_state_publisher_node") {}

void ContactStatePublisher::init() {
  buffer_ = std::make_unique<tf2_ros::Buffer>(this->get_clock());
  listener_ = std::make_shared<tf2_ros::TransformListener>(*buffer_);

  // Declare and get string parameters directly (avoid shared_from_this quirks)
  auto getStr = [this](const std::string& name,
                        const std::string& default_val) {
    this->declare_parameter<std::string>(name, default_val);
    return this->get_parameter(name).as_string();
  };
  auto getDouble = [this](const std::string& name, double default_val) {
    this->declare_parameter<double>(name, default_val);
    return this->get_parameter(name).as_double();
  };

  std::string grf_topic = getStr("topics.state.grfs", "state/grfs");
  std::string toe0_topic =
      getStr("topics.gazebo.toe0_contact_state", "gazebo/toe0_contact_state");
  std::string toe1_topic =
      getStr("topics.gazebo.toe1_contact_state", "gazebo/toe1_contact_state");
  std::string toe2_topic =
      getStr("topics.gazebo.toe2_contact_state", "gazebo/toe2_contact_state");
  std::string toe3_topic =
      getStr("topics.gazebo.toe3_contact_state", "gazebo/toe3_contact_state");
  update_rate_ = getDouble("contact_state_publisher.update_rate", 500.0);

  auto qos = rclcpp::QoS(1).reliable();

  toe0_contact_state_sub =
      this->create_subscription<gazebo_msgs::msg::ContactsState>(
          toe0_topic, qos,
          [this](const gazebo_msgs::msg::ContactsState& msg) {
            contactStateCallback(msg, 0);
          });
  toe1_contact_state_sub =
      this->create_subscription<gazebo_msgs::msg::ContactsState>(
          toe1_topic, qos,
          [this](const gazebo_msgs::msg::ContactsState& msg) {
            contactStateCallback(msg, 1);
          });
  toe2_contact_state_sub =
      this->create_subscription<gazebo_msgs::msg::ContactsState>(
          toe2_topic, qos,
          [this](const gazebo_msgs::msg::ContactsState& msg) {
            contactStateCallback(msg, 2);
          });
  toe3_contact_state_sub =
      this->create_subscription<gazebo_msgs::msg::ContactsState>(
          toe3_topic, qos,
          [this](const gazebo_msgs::msg::ContactsState& msg) {
            contactStateCallback(msg, 3);
          });

  grf_pub_ = this->create_publisher<quad_msgs::msg::GRFArray>(grf_topic, 1);

  grf_array_msg_.vectors.resize(num_feet_);
  grf_array_msg_.points.resize(num_feet_);
  grf_array_msg_.contact_states.resize(num_feet_);
  ready_to_publish_ = false;

  auto period = std::chrono::duration<double>(1.0 / update_rate_);
  timer_ = this->create_wall_timer(
      period, std::bind(&ContactStatePublisher::publishContactState, this));
}

void ContactStatePublisher::contactStateCallback(
    const gazebo_msgs::msg::ContactsState& msg, const int toe_idx) {
  std::string terrain_name = "mesh_terrain";
  std::string toe_collision_names[4] = {"toe0_collision", "toe1_collision",
                                        "toe2_collision", "toe3_collision"};
  std::string toe_string = toe_collision_names[toe_idx];

  std::string ns = this->get_namespace();
  if (!ns.empty() && ns[0] == '/') {
    ns = ns.substr(1);
  }

  std::string toe_transform_names[4] = {
      ns + "_ground_truth/toe0", ns + "_ground_truth/toe1",
      ns + "_ground_truth/toe2", ns + "_ground_truth/toe3"};

  grf_array_msg_.vectors[toe_idx].x = 0.0;
  grf_array_msg_.vectors[toe_idx].y = 0.0;
  grf_array_msg_.vectors[toe_idx].z = 0.0;
  grf_array_msg_.points[toe_idx].x = 0.0;
  grf_array_msg_.points[toe_idx].y = 0.0;
  grf_array_msg_.points[toe_idx].z = 0.0;
  grf_array_msg_.contact_states[toe_idx] = false;

  for (size_t i = 0; i < msg.states.size(); i++) {
    std::string str_toe = msg.states[i].collision1_name;
    std::string str_terrain = msg.states[i].collision2_name;
    std::size_t found_toe = str_toe.find(toe_string);
    std::size_t found_terrain = str_terrain.find(terrain_name);

    if ((found_toe != std::string::npos) &&
        (found_terrain != std::string::npos)) {
      grf_array_msg_.vectors[toe_idx].x = msg.states[i].total_wrench.force.x;
      grf_array_msg_.vectors[toe_idx].y = msg.states[i].total_wrench.force.y;
      grf_array_msg_.vectors[toe_idx].z = msg.states[i].total_wrench.force.z;

      for (size_t j = 0; j < msg.states[i].contact_positions.size(); j++) {
        grf_array_msg_.points[toe_idx].x +=
            msg.states[i].contact_positions[j].x;
        grf_array_msg_.points[toe_idx].y +=
            msg.states[i].contact_positions[j].y;
        grf_array_msg_.points[toe_idx].z +=
            msg.states[i].contact_positions[j].z;
      }

      grf_array_msg_.points[toe_idx].x /=
          msg.states[i].contact_positions.size();
      grf_array_msg_.points[toe_idx].y /=
          msg.states[i].contact_positions.size();
      grf_array_msg_.points[toe_idx].z /=
          msg.states[i].contact_positions.size();

      grf_array_msg_.contact_states[toe_idx] = true;
      break;
    }
  }

  geometry_msgs::msg::TransformStamped transformsStamped;
  try {
    transformsStamped = buffer_->lookupTransform(
        "map", toe_transform_names[toe_idx], tf2::TimePointZero);
  } catch (tf2::TransformException& ex) {
    RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000,
                         "%s", ex.what());
    ready_to_publish_ = false;
    return;
  }

  transformsStamped.transform.translation.x = 0;
  transformsStamped.transform.translation.y = 0;
  transformsStamped.transform.translation.z = 0;

  tf2::doTransform(grf_array_msg_.vectors[toe_idx],
                   grf_array_msg_.vectors[toe_idx], transformsStamped);

  ready_to_publish_ = true;
}

void ContactStatePublisher::publishContactState() {
  if (!ready_to_publish_) {
    return;
  }
  grf_array_msg_.header.stamp = this->now();
  grf_pub_->publish(grf_array_msg_);
}
