import serial
import pygame
import math
import time

PORT = "/dev/tty.usbmodem1101"
BAUD = 115200

ser = serial.Serial(PORT, BAUD, timeout=0.1)
time.sleep(2)

pygame.init()

WIDTH = 800
HEIGHT = 600
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("HW17 Haptic Paddle Graphics")

font = pygame.font.SysFont(None, 32)
clock = pygame.time.Clock()

center_x = WIDTH // 2
center_y = HEIGHT // 2
arm_len = 180

angle_deg = 0.0
force_raw = 0.0
force_filtered = 0.0

# 用来画 force bar 的初始 offset
force_zero = None

running = True

while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    try:
        line = ser.readline().decode("utf-8").strip()

        if line:
            parts = line.split(",")

            # Pico 格式:
            # time_ms,force_raw,force_filtered,angle_deg
            if len(parts) == 4:
                time_ms = float(parts[0])
                force_raw = float(parts[1])
                force_filtered = float(parts[2])
                angle_deg = float(parts[3])

                if force_zero is None:
                    force_zero = force_filtered

    except:
        pass

    screen.fill((240, 240, 240))

    # ---------------- Draw paddle ----------------
    theta = math.radians(angle_deg)

    end_x = center_x + arm_len * math.cos(theta)
    end_y = center_y - arm_len * math.sin(theta)

    pygame.draw.circle(screen, (0, 0, 0), (center_x, center_y), 10)
    pygame.draw.line(
        screen,
        (30, 80, 200),
        (center_x, center_y),
        (end_x, end_y),
        8
    )
    pygame.draw.circle(screen, (30, 80, 200), (int(end_x), int(end_y)), 18)

    # ---------------- Draw force bar ----------------
    if force_zero is not None:
        force_display = force_filtered - force_zero
    else:
        force_display = 0

    # 缩放比例，太大或太小可以改这个
    force_scale = 0.002
    bar_len = force_display * force_scale

    # 限制长度
    if bar_len > 250:
        bar_len = 250
    if bar_len < -250:
        bar_len = -250

    bar_x = 100
    bar_y = 500

    pygame.draw.line(screen, (0, 0, 0), (bar_x, bar_y), (bar_x + 500, bar_y), 3)
    pygame.draw.line(screen, (200, 40, 40), (bar_x + 250, bar_y),
                     (bar_x + 250 + bar_len, bar_y), 12)
    pygame.draw.circle(screen, (0, 0, 0), (bar_x + 250, bar_y), 6)

    # ---------------- Text ----------------
    text1 = font.render(f"Angle: {angle_deg:.2f} deg", True, (0, 0, 0))
    text2 = font.render(f"Force raw: {force_raw:.0f}", True, (0, 0, 0))
    text3 = font.render(f"Force filtered: {force_filtered:.2f}", True, (0, 0, 0))
    text4 = font.render("Blue arm = paddle angle", True, (0, 0, 0))
    text5 = font.render("Red bar = force direction/magnitude", True, (0, 0, 0))

    screen.blit(text1, (30, 30))
    screen.blit(text2, (30, 65))
    screen.blit(text3, (30, 100))
    screen.blit(text4, (30, 140))
    screen.blit(text5, (30, 175))

    pygame.display.flip()
    clock.tick(30)

ser.close()
pygame.quit()