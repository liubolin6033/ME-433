#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"

#include "bsp/board.h"
#include "tusb.h"
#include "usb_descriptors.h"

#define I2C_PORT        i2c0
#define I2C_SDA_PIN     4
#define I2C_SCL_PIN     5

#define MPU6050_ADDR    0x68
#define MPU6050_PWR1    0x6B
#define MPU6050_ACCEL   0x3B

#define BUTTON_PIN      19
#define MODE_LED_PIN    16

enum {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED     = 1000,
  BLINK_SUSPENDED   = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

typedef enum {
  MODE_REGULAR = 0,
  MODE_REMOTE  = 1
} mouse_mode_t;

static mouse_mode_t current_mode = MODE_REGULAR;

static const int8_t circle_dx[16] = { 4, 4, 3, 2, 0,-2,-3,-4,-4,-4,-3,-2, 0, 2, 3, 4 };
static const int8_t circle_dy[16] = { 0, 2, 3, 4, 4, 4, 3, 2, 0,-2,-3,-4,-4,-4,-3,-2 };
static uint8_t circle_idx = 0;

void usb_led_blinking_task(void);
void hid_task(void);
void mode_led_task(void);

static void mpu6050_init(void);
static void mpu6050_read_raw(int16_t *ax, int16_t *ay, int16_t *az);
static bool button_pressed(void);
static int8_t accel_to_delta(int16_t a);


void tud_mount_cb(void) {
  blink_interval_ms = BLINK_MOUNTED;
}

void tud_umount_cb(void) {
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

void tud_suspend_cb(bool remote_wakeup_en) {
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

void tud_resume_cb(void) {
  blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
}

static void mpu6050_init(void) {
  uint8_t buf[2] = {MPU6050_PWR1, 0x00};
  i2c_write_blocking(I2C_PORT, MPU6050_ADDR, buf, 2, false);
  sleep_ms(100);
}

static void mpu6050_read_raw(int16_t *ax, int16_t *ay, int16_t *az) {
  uint8_t reg = MPU6050_ACCEL;
  uint8_t data[6];

  i2c_write_blocking(I2C_PORT, MPU6050_ADDR, &reg, 1, true);
  i2c_read_blocking(I2C_PORT, MPU6050_ADDR, data, 6, false);

  *ax = (int16_t)((data[0] << 8) | data[1]);
  *ay = (int16_t)((data[2] << 8) | data[3]);
  *az = (int16_t)((data[4] << 8) | data[5]);
}


static bool button_pressed(void) {
  static bool last_state = true; // pull-up idle high
  static absolute_time_t last_press_time = {0};

  bool state = gpio_get(BUTTON_PIN);

  if (last_state == true && state == false) {
    if (absolute_time_diff_us(last_press_time, get_absolute_time()) > 200000) {
      last_press_time = get_absolute_time();
      last_state = state;
      return true;
    }
  }

  last_state = state;
  return false;
}

static int8_t accel_to_delta(int16_t a) {
  int16_t abs_a = (a >= 0) ? a : -a;

  if (abs_a < 3500) {
    return 0;
  } else if (abs_a < 6500) {
    return (a > 0) ? 1 : -1;
  } else if (abs_a < 10000) {
    return (a > 0) ? 3 : -3;
  } else {
    return (a > 0) ? 5 : -5;
  }
}

int main(void) {
  board_init();
  stdio_init_all();

  gpio_init(BUTTON_PIN);
  gpio_set_dir(BUTTON_PIN, GPIO_IN);
  gpio_pull_up(BUTTON_PIN);

  gpio_init(MODE_LED_PIN);
  gpio_set_dir(MODE_LED_PIN, GPIO_OUT);
  gpio_put(MODE_LED_PIN, 1);

  i2c_init(I2C_PORT, 400000);
  gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
  gpio_pull_up(I2C_SDA_PIN);
  gpio_pull_up(I2C_SCL_PIN);

  sleep_ms(100);
  mpu6050_init();

  tud_init(0);

  while (1) {
    tud_task();
    hid_task();
    usb_led_blinking_task();
    mode_led_task();
  }

  return 0;
}

void hid_task(void) {
  static uint32_t start_ms = 0;

  if (board_millis() - start_ms < 10) return;
  start_ms += 10;

  if (!tud_hid_ready()) return;

  if (button_pressed()) {
    current_mode = (current_mode == MODE_REGULAR) ? MODE_REMOTE : MODE_REGULAR;
  }

  int8_t dx = 0;
  int8_t dy = 0;

  if (current_mode == MODE_REGULAR) {
    int16_t ax, ay, az;
    mpu6050_read_raw(&ax, &ay, &az);

    dx = -accel_to_delta(ax);
    dy = accel_to_delta(ay);
  } else {
    dx = circle_dx[circle_idx];
    dy = circle_dy[circle_idx];
    circle_idx = (circle_idx + 1) % 16;
  }

  tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, dx, dy, 0, 0);
}

void usb_led_blinking_task(void) {
  static uint32_t start_ms = 0;
  static bool led_state = false;

  if (board_millis() - start_ms < blink_interval_ms) return;
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = !led_state;
}

void mode_led_task(void) {
  static uint32_t last_blink_ms = 0;
  static bool led_state = true;

  if (current_mode == MODE_REGULAR) {
    gpio_put(MODE_LED_PIN, 1);  
  } else {
    if (board_millis() - last_blink_ms >= 250) {
      last_blink_ms = board_millis();
      led_state = !led_state;
      gpio_put(MODE_LED_PIN, led_state);
    }
  }
}