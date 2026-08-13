#ifndef GLOBAL_BODY_PLANNER_H
#define GLOBAL_BODY_PLANNER_H

#include <geometry_msgs/msg/point_stamped.hpp>
#include <grid_map_core/grid_map_core.hpp>
#include <grid_map_msgs/msg/grid_map.hpp>
#include <grid_map_ros/GridMapRosConverter.hpp>
#include <grid_map_ros/grid_map_ros.hpp>
#include <nav_msgs/msg/path.hpp>
#include <quad_msgs/msg/robot_plan.hpp>
#include <quad_msgs/msg/robot_state.hpp>
#include <quad_utils/ros_utils.h>
#include <rclcpp/rclcpp.hpp>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <visualization_msgs/msg/marker_array.hpp>

#include "global_body_planner/gbpl.h"
#include "global_body_planner/global_body_plan.h"

using namespace planning_utils;

//! A global body planning class for legged robots
//! 用于足式机器人的全局机身规划类
/*!
   GlobalBodyPlanner is a container for all of the logic utilized in the global
   body planning node. This algorithm requires an height map of the terrain as a
   GridMap message type, and will publish the global body plan as a BodyPlan
   message over a topic. It will also publish the discrete states used by the
   planner (from which the full path is interpolated).
   GlobalBodyPlanner 封装了全局机身规划节点中用到的所有逻辑。该算法需要地形高
   程图（以 GridMap 消息类型提供），并将全局机身规划结果以 BodyPlan 消息的形式
   通过话题发布。它还会发布规划器所使用的离散状态（完整路径由这些状态插值得到）。
*/
class GlobalBodyPlanner {
 public:
  /**
   * @brief Constructor for GlobalBodyPlanner Class
   * @brief GlobalBodyPlanner 类的构造函数
   * @param[in] nh Node handle
   * @param[in] nh 共享指针
   * @return Constructed object of type GlobalBodyPlanner
   * @return 返回 GlobalBodyPlanner 类型的已构造对象
   */
  GlobalBodyPlanner(rclcpp::Node::SharedPtr node);

  /**
   * @brief Call the correct planning class and compute statistics
   * @brief 调用正确的规划器类并计算统计信息
   * @return Boolean for success of the planner
   * @return 规划是否成功的布尔值
   */
  bool callPlanner();

  /**
   * @brief Primary work function in class, called in node file for this
   * component
   * @brief 类中的主要工作函数，由该组件的节点文件调用
   */
  void spin();

 private:
  /**
   * @brief Callback function to handle new terrain map data
   * @brief 处理新地形地图数据的回调函数
   * @param[in] msg the message contining map data
   * @param[in] msg 包含地图数据的消息
   */
  void terrainMapCallback(const grid_map_msgs::msg::GridMap::SharedPtr msg);

  /**
   * @brief Callback function to handle new robot state data
   * @brief 处理新机器人状态数据的回调函数
   * @param[in] msg the message contining robot state data
   * @param[in] msg 包含机器人状态数据的消息
   */
  void robotStateCallback(const quad_msgs::msg::RobotState::SharedPtr msg);

  /**
   * @brief Callback function to handle new goal state
   * @brief 处理新目标状态的回调函数
   * @param[in] msg the message contining the goal state
   * @param[in] msg 包含目标状态信息的消息
   */
  void goalStateCallback(const geometry_msgs::msg::PointStamped::SharedPtr msg);

  /**
   * @brief Trigger a reset event
   * @brief 触发一次重置事件
   */
  void triggerReset();

  /**
   * @brief Initialize the planner by clearing out old plan data and setting the
   * start state
   * @brief 通过清空旧的规划数据并设置起始状态来初始化规划器
   * @return Index of the current plan from which to being the new plan (zero if
   * fully replanning)
   * @return 新规划所基于的当前规划索引（若完全重新规划则为 0）
   */
  int initPlanner();

  /**
   * @brief Clear all data in plan member variables
   * @brief 清空规划成员变量中的所有数据
   */
  void clearPlan();

  /**
   * @brief Update the body plan with the current plan
   * @brief 使用当前规划更新机身规划
   * @param[in] t Time of state in trajectory
   * @param[in] t 轨迹中该状态对应的时间
   * @param[in] body_state Body state
   * @param[in] body_state 机身状态
   * @param[in] grf GRF applied to body
   * @param[in] grf 作用在机身上的地面反作用力
   * @param[in] body_plan_msg Body plan message
   * @param[in] body_plan_msg 机身规划消息
   */
  void addStateAndGRFToMsg(double t, int plan_index, FullState body_state,
                           GRF grf, int primitive_id,
                           quad_msgs::RobotPlan &body_plan_msg);

  /**
   * @brief Publish the current body plan
   * @brief 发布当前的机身规划
   */
  void publishPlan();

  /**
   * @brief Wait until map and state messages have been received and processed
   * @brief 等待直到已接收并处理完地图和状态消息
   */
  void waitForData();

  /**
   * @brief Call the planner repeatedly until startup_delay has been reached
   * then return
   * @brief 反复调用规划器，直到达到启动延迟时间后再返回
   */
  void getInitialPlan();

  /**
   * @brief Set the start state to be used by the next planning call
   * @brief 设置下一次规划调用所使用的起始状态
   */
  void setStartState();

  /**
   * @brief Set the goal state to be used by the next planning call
   * @brief 设置下一次规划调用所使用的目标状态
   */
  void setGoalState();

  /**
   * @brief Publish the current plan if updated
   * @brief 若规划已更新则发布当前规划
   */
  void publishCurrentPlan();

  /// Subscriber for terrain map messages
  /// 地形地图消息的订阅者
  rclcpp::Subscription<grid_map_msgs::msg::GridMap>::SharedPtr terrain_map_sub_;

  /// Subscriber for robot state messages
  /// 机器人状态消息的订阅者
  rclcpp::Subscription<quad_msgs::msg::RobotState>::SharedPtr robot_state_sub_;

  /// Subscriber for goal state messages
  /// 目标状态消息的订阅者
  rclcpp::Subscription<geometry_msgs::msg::PointStamped>::SharedPtr goal_state_sub_;

  /// Publisher for body plan messages
  /// 机身规划消息的发布者
  rclcpp::Publisher<quad_msgs::msg::RobotPlan>::SharedPtr body_plan_pub_;

  /// Publisher for discrete states in body plan messages
  /// 机身规划消息中离散状态的发布者
  rclcpp::Publisher<quad_msgs::msg::RobotPlan>::SharedPtr discrete_body_plan_pub_;

  /// Publisher for the planning tree
  /// 规划树的发布者
  rclcpp::Publisher<visualization_msgs::msg::MarkerArray>::SharedPtr tree_pub_;

  /// Topic name for terrain map (needed to ensure data has been received)
  /// 地形地图的话题名（用于确认已接收到数据）
  std::string terrain_map_topic_;

  /// Topic name for robot state data (needed to ensure data has been received)
  /// 机器人状态数据的话题名（用于确认已接收到数据）
  std::string robot_state_topic_;

  /// Nodehandle to pub to and sub from
  /// 用于发布和订阅的节点句柄
  rclcpp::Node::SharedPtr node_;

  /// Update rate for sending and receiving data;
  /// 发送和接收数据的更新频率
  double update_rate_;

  /// Number of times to call the planner
  /// 调用规划器的次数
  int num_calls_;

  /// Max time to let the algorithm search
  /// 算法允许搜索的最大时间
  double max_planning_time_;

  /// Handle for the map frame
  /// 地图坐标系的标识
  std::string map_frame_;

  /// Plan data for the most recently computed plan (may be suboptimal)
  /// 最近一次计算的规划数据（可能不是最优解）
  GlobalBodyPlan newest_plan_;

  /// Plan data for the plan currently being executed
  /// 当前正在执行的规划数据
  GlobalBodyPlan current_plan_;

  /// Starting state for planner call
  /// 规划调用的起始状态
  FullState start_state_;

  /// Goal state for planner
  /// 规划器的目标状态
  FullState goal_state_;

  /// Current robot state
  /// 当前机器人状态
  FullState robot_state_;

  /// goal_state_msg_
  /// 目标状态消息
  geometry_msgs::msg::PointStamped::SharedPtr goal_state_msg_;

  /// Starting time for planner call during replans relative to t_plan_[0]
  /// 重新规划时规划调用的起始时间（相对于 t_plan_[0]）
  double replan_start_time_;

  /// Horizon to commit to (replan from the next state after this horizon)
  /// 承诺执行的时域长度（超过此后从该下一个状态重新规划）
  double committed_horizon_;

  /// Threshold of state error to trigger replanning
  /// 触发重新规划的状态误差阈值
  double pos_error_threshold_;

  /// Flag to determine if the planner needs to restart planning from the robot
  /// state
  /// 标志位，指示规划器是否需要从机器人状态重新开始规划
  bool restart_flag_;

  /// Sequence of discrete states in the plan
  /// 规划中离散状态的序列
  std::vector<State> state_sequence_;

  /// Sequence of discrete actions in the plan
  /// 规划中离散动作的序列
  std::vector<Action> action_sequence_;

  /// Vector of cost instances in each planning call
  /// 每次规划调用中代价实例的向量
  std::vector<double> cost_vector_;

  /// Vector of time instances of cost data for each planning call
  /// 每次规划调用中代价数据对应时刻的向量
  std::vector<double> cost_vector_times_;

  /// Vector of solve times for each planning call
  /// 每次规划调用求解耗时的向量
  std::vector<double> solve_time_info_;

  /// Vector of number of vertices for each planning call
  /// 每次规划调用生成顶点数的向量
  std::vector<int> vertices_generated_info_;

  /// Planner config
  /// 规划器配置
  PlannerConfig planner_config_;

  /// Delay after plan reset before publishing and refining
  /// 规划重置后、发布与优化前的延迟时间
  double reset_publish_delay_;

  /// Boolean for whether replanning is allowed
  /// 是否允许重新规划的布尔值
  bool replanning_allowed_;

  /// Timestep for interpolation
  /// 插值使用的时间步长
  double dt_;

  /// ID for status of planner
  /// 规划器状态的 ID
  int planner_status_;

  /// Index of active plan from which to begin refinement
  /// 用于开始优化的活动规划索引
  int start_index_;

  /// Planning status ID for resetting (start from scratch)
  /// 重置（从头开始）的规划状态 ID
  static const int RESET = 0;

  /// Planning status ID for refining (optimize current plan)
  /// 优化（对当前规划进行优化）的规划状态 ID
  static const int REFINE = 1;

  /// Time at which reset began
  /// 重置开始的时间
  rclcpp::Time reset_time_;

  /// Boolean for whether to publish the new plan after the reset delay has
  /// occured
  /// 重置延迟结束后是否发布新规划的布尔值
  bool publish_after_reset_delay_;

  /// Timestamp for t=0 of global plan
  /// 全局规划 t=0 时刻的时间戳
  rclcpp::Time global_plan_timestamp_;
};

#endif  // GLOBAL_BODY_PLANNER_H
