#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C defines
#define I2C_PORT i2c0
#define I2C_SDA 4
#define I2C_SCL 5

// MPU6050
#define MPU6050_ADDR 0x68
#define PWR_MGMT_1 0x6B
#define ACCEL_XOUT_H 0x3B

// Button
#define BUTTON_PIN 26

// 写寄存器
void mpu6050_write(uint8_t reg, uint8_t value) {
    uint8_t buf[2] = {reg, value};
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, buf, 2, false);
}

// 读寄存器
void mpu6050_read(uint8_t reg, uint8_t *buf, uint8_t len) {
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, MPU6050_ADDR, buf, len, false);
}

// 拼接高低字节
int16_t combine_bytes(uint8_t high, uint8_t low) {
    return (int16_t)((high << 8) | low);
}

int main()
{
    stdio_init_all();

    // I2C 初始化
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // 按钮初始化（GP26）
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    sleep_ms(1000); // 等 USB 稳定

    // 唤醒 MPU6050
    mpu6050_write(PWR_MGMT_1, 0x00);

    while (true) {
        uint8_t data[6];

        // 读取加速度
        mpu6050_read(ACCEL_XOUT_H, data, 6);

        int16_t ax_raw = combine_bytes(data[0], data[1]);
        int16_t ay_raw = combine_bytes(data[2], data[3]);
        int16_t az_raw = combine_bytes(data[4], data[5]);

        // 转换为 g
        float ax = ax_raw / 16384.0;
        float ay = ay_raw / 16384.0;
        float az = az_raw / 16384.0;

        // 读取按钮
        int button = gpio_get(BUTTON_PIN);

        // 串口输出（Python 会用）
        printf("%.2f,%.2f,%.2f,%d\n", ax, ay, az, button);

        sleep_ms(30);
    }
}