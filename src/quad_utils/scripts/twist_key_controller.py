#!/usr/bin/env python3
import pygame
import sys
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from std_msgs.msg import UInt8


class TwistKeyController(Node):
    """Keyboard teleoperation node using pygame for ROS2."""

    def __init__(self):
        super().__init__('twist_key_controller')

        self.twist_pub = self.create_publisher(Twist, '/cmd_vel', 1)
        self.control_mode_pub = self.create_publisher(UInt8, '/control/mode', 1)

        self.lin_vel = 0.5
        self.ang_vel = 0.2

        pygame.init()
        self.screen = pygame.display.set_mode((300, 200))
        pygame.display.set_caption('Twist keyboard controller')

        self.timer = self.create_timer(1.0 / 30.0, self.timer_callback)

    def timer_callback(self):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                rclpy.shutdown()
                return

        keys_pressed = pygame.key.get_pressed()

        # Collect meta keys
        alt_pressed = keys_pressed[pygame.K_RALT] or keys_pressed[pygame.K_LALT]
        ctrl_pressed = keys_pressed[pygame.K_RCTRL] or keys_pressed[pygame.K_LCTRL]
        shift_pressed = keys_pressed[pygame.K_RSHIFT] or keys_pressed[pygame.K_LSHIFT]
        space_pressed = keys_pressed[pygame.K_SPACE]

        twist_vel = [0, 0]
        twist_vel[0] -= keys_pressed[pygame.K_DOWN]
        twist_vel[0] += keys_pressed[pygame.K_UP]
        twist_vel[1] -= keys_pressed[pygame.K_LEFT]
        twist_vel[1] += keys_pressed[pygame.K_RIGHT]

        control_mode = -1
        if keys_pressed[pygame.K_0]:
            control_mode = 0
        elif keys_pressed[pygame.K_1]:
            control_mode = 1
        elif keys_pressed[pygame.K_2]:
            control_mode = 2

        if space_pressed:
            self.twist_pub.publish(Twist())
        elif twist_vel[0] != 0 or twist_vel[1] != 0:
            twist_cmd = Twist()
            twist_cmd.linear.x = self.lin_vel * twist_vel[0]

            # Hold shift to strafe
            if shift_pressed:
                twist_cmd.linear.y = self.lin_vel * twist_vel[1]
            else:
                twist_cmd.angular.z = -self.ang_vel * twist_vel[1]

            self.twist_pub.publish(twist_cmd)

        if control_mode != -1:
            self.control_mode_pub.publish(UInt8(data=control_mode))


def main():
    rclpy.init()
    node = TwistKeyController()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
