# Person-Following Ambulatory Assistance Companion Walker
Arduino code for the Person-Following Ambulatory Assistance Companion Walker
## ➕ Features:
- Receives ROS Twist messages through the ROS network for automated movement.
- Reads joystick input for manual control over the mecanum wheels.
- Static speed replaced with incrementing/decrementing speeds (to a set limit) for smoother acceleration/deceleration.

## ➖ Limitations:
- Actually rotates on left/right joystick movement. To be fixed to sideward+rotate movement.
