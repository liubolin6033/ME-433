import time
import pwmio
import board

servo = pwmio.PWMOut(board.GP16, frequency=50, duty_cycle=0)

def set_servo_angle(angle):
    if angle < 0:
        angle = 0
    if angle > 180:
        angle = 180

    pulse_ms = 0.5 + (angle / 180.0) * 2.0
    duty = int((pulse_ms / 20.0) * 65535)

    servo.duty_cycle = duty

while True:
    for angle in range(0, 181, 2):
        set_servo_angle(angle)
        time.sleep(0.02)

    for angle in range(180, -1, -2):
        set_servo_angle(angle)
        time.sleep(0.02)