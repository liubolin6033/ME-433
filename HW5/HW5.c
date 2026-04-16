#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"

#define I2C_PORT i2c0
#define I2C_SDA 4
#define I2C_SCL 5

#define OLED_WIDTH 128
#define OLED_HEIGHT 32
#define OLED_CENTER_X 64
#define OLED_CENTER_Y 16

#define MPU6050_ADDR 0x68

#define GYRO_CONFIG  0x1B
#define ACCEL_CONFIG 0x1C
#define PWR_MGMT_1   0x6B

#define ACCEL_XOUT_H 0x3B
#define WHO_AM_I     0x75

#define AX_OFFSET 884
#define AY_OFFSET 372

uint8_t read_register(uint8_t reg) {
    uint8_t val;
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, MPU6050_ADDR, &val, 1, false);
    return val;
}

void write_register(uint8_t reg, uint8_t val) {
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = val;
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, buf, 2, false);
}

void mpu6050_init() {
    write_register(PWR_MGMT_1, 0x00);
    write_register(ACCEL_CONFIG, 0x00);
    write_register(GYRO_CONFIG, 0x18);
}

int16_t combine_bytes(uint8_t high, uint8_t low) {
    return (int16_t)((high << 8) | low);
}

void mpu6050_read_all(int16_t *ax, int16_t *ay, int16_t *az,
                      int16_t *temp_raw,
                      int16_t *gx, int16_t *gy, int16_t *gz) {
    uint8_t reg = ACCEL_XOUT_H;
    uint8_t data[14];

    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, MPU6050_ADDR, data, 14, false);

    *ax = combine_bytes(data[0], data[1]);
    *ay = combine_bytes(data[2], data[3]);
    *az = combine_bytes(data[4], data[5]);
    *temp_raw = combine_bytes(data[6], data[7]);
    *gx = combine_bytes(data[8], data[9]);
    *gy = combine_bytes(data[10], data[11]);
    *gz = combine_bytes(data[12], data[13]);
}

void draw_line(int x0, int y0, int x1, int y1, int color) {
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while (1) {
        if (x0 >= 0 && x0 < OLED_WIDTH && y0 >= 0 && y0 < OLED_HEIGHT) {
            ssd1306_drawPixel(x0, y0, color);
        }

        if (x0 == x1 && y0 == y1) {
            break;
        }

        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void draw_crosshair() {
    ssd1306_drawPixel(OLED_CENTER_X, OLED_CENTER_Y, 1);
    ssd1306_drawPixel(OLED_CENTER_X - 1, OLED_CENTER_Y, 1);
    ssd1306_drawPixel(OLED_CENTER_X + 1, OLED_CENTER_Y, 1);
    ssd1306_drawPixel(OLED_CENTER_X, OLED_CENTER_Y - 1, 1);
    ssd1306_drawPixel(OLED_CENTER_X, OLED_CENTER_Y + 1, 1);
}

int main() {
    stdio_init_all();

    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    sleep_ms(2000);

    ssd1306_setup();
    ssd1306_clear();
    ssd1306_update();

    uint8_t who = read_register(WHO_AM_I);
    printf("WHO_AM_I = 0x%X\n", who);

    if (who != 0x68 && who != 0x98) {
        while (1) {
            printf("MPU6050 not found!\n");
            sleep_ms(1000);
        }
    }

    mpu6050_init();

    while (1) {
        int16_t ax, ay, az, temp_raw, gx, gy, gz;
        float temp_c;

        mpu6050_read_all(&ax, &ay, &az, &temp_raw, &gx, &gy, &gz);
        temp_c = temp_raw / 340.0f + 36.53f;

        printf("AX:%6d AY:%6d AZ:%6d | ", ax, ay, az);
        printf("GX:%6d GY:%6d GZ:%6d | ", gx, gy, gz);
        printf("Temp: %.2f C\n", temp_c);

        int ax_cal = ax - AX_OFFSET;
        int ay_cal = ay - AY_OFFSET;

        int dx = ax_cal / 1000;
        int dy = ay_cal / 1000;

        int x1 = OLED_CENTER_X - dx;
        int y1 = OLED_CENTER_Y + dy;

        if (x1 < 0) x1 = 0;
        if (x1 > OLED_WIDTH - 1) x1 = OLED_WIDTH - 1;
        if (y1 < 0) y1 = 0;
        if (y1 > OLED_HEIGHT - 1) y1 = OLED_HEIGHT - 1;

        ssd1306_clear();
        draw_crosshair();
        draw_line(OLED_CENTER_X, OLED_CENTER_Y, x1, y1, 1);
        ssd1306_update();

        sleep_ms(10);
    }
}