import pgzrun
import serial
import threading

WIDTH = 800
HEIGHT = 500

SERIAL_PORT = "/dev/tty.usbmodem1101"
BAUD_RATE = 115200

ax = 0
ay = 0
az = 0
button = 1

ball_x = WIDTH // 2
ball_y = HEIGHT // 2
ball_radius = 25
score = 0


def read_serial():
    global ax, ay, az, button

    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)

    while True:
        line = ser.readline().decode(errors="ignore").strip()
        parts = line.split(",")

        if len(parts) == 4:
            try:
                ax = float(parts[0])
                ay = float(parts[1])
                az = float(parts[2])
                button = int(parts[3])
            except:
                pass


threading.Thread(target=read_serial, daemon=True).start()


def update():
    global ball_x, ball_y, ball_radius, score

    move_x = -ax
    move_y = ay

    if abs(move_x) < 0.08:
        move_x = 0
    if abs(move_y) < 0.08:
        move_y = 0

    ball_x += move_x * 10
    ball_y += move_y * 10

    ball_x = max(ball_radius, min(WIDTH - ball_radius, ball_x))
    ball_y = max(ball_radius, min(HEIGHT - ball_radius, ball_y))

    if button == 0:
        ball_radius = 50
        score += 1
    else:
        ball_radius = 25


def draw():
    screen.clear()

    screen.draw.text("HW10 Game",
                     center=(WIDTH // 2, 40),
                     fontsize=40,
                     color="white")

    screen.draw.filled_circle((int(ball_x), int(ball_y)),
                              ball_radius,
                              "cyan")

    screen.draw.text(f"ax: {ax:.2f}", (30, HEIGHT - 120), fontsize=28, color="white")
    screen.draw.text(f"ay: {ay:.2f}", (30, HEIGHT - 90), fontsize=28, color="white")
    screen.draw.text(f"az: {az:.2f}", (30, HEIGHT - 60), fontsize=28, color="white")
    

pgzrun.go()