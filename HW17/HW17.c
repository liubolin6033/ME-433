#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// ---------------- AS5600 ----------------
#define I2C_PORT i2c0
#define I2C_SDA 4
#define I2C_SCL 5

#define AS5600_ADDR 0x36
#define AS5600_ANGLE_REG 0x0C

// ---------------- HX711 ----------------
#define HX711_SCK 15
#define HX711_DT  14

#define ALPHA 0.10f

uint16_t read_as5600_raw(void)
{
    uint8_t reg = AS5600_ANGLE_REG;
    uint8_t buf[2];

    i2c_write_blocking(I2C_PORT, AS5600_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, AS5600_ADDR, buf, 2, false);

    uint16_t raw = ((uint16_t)buf[0] << 8) | buf[1];
    raw &= 0x0FFF;

    return raw;
}

void hx711_init(void)
{
    gpio_init(HX711_SCK);
    gpio_set_dir(HX711_SCK, GPIO_OUT);
    gpio_put(HX711_SCK, 0);

    gpio_init(HX711_DT);
    gpio_set_dir(HX711_DT, GPIO_IN);
}

int32_t hx711_read(void)
{
    uint32_t raw = 0;

    while (gpio_get(HX711_DT)) {
        tight_loop_contents();
    }

    for (int i = 0; i < 24; i++) {
        gpio_put(HX711_SCK, 1);
        sleep_us(5);

        raw <<= 1;

        if (gpio_get(HX711_DT)) {
            raw |= 1;
        }

        gpio_put(HX711_SCK, 0);
        sleep_us(5);
    }

    gpio_put(HX711_SCK, 1);
    sleep_us(5);
    gpio_put(HX711_SCK, 0);
    sleep_us(5);

    if (raw & 0x800000) {
        raw |= 0xFF000000;
    }

    return (int32_t)raw;
}

int main(void)
{
    stdio_init_all();

    hx711_init();

    i2c_init(I2C_PORT, 100 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    sleep_ms(2000);

    printf("Test 4: HX711 + AS5600 merged\n");
    printf("time_ms,force_raw,force_filtered,angle_deg\n");

    absolute_time_t start_time = get_absolute_time();

    int32_t force_raw = hx711_read();
    float force_filtered = (float)force_raw;

    while (true) {
        force_raw = hx711_read();

        force_filtered = ALPHA * (float)force_raw
                       + (1.0f - ALPHA) * force_filtered;

        uint16_t angle_raw = read_as5600_raw();
        float angle_deg = angle_raw * 360.0f / 4096.0f;

        int64_t time_ms =
            absolute_time_diff_us(start_time, get_absolute_time()) / 1000;

        printf("%lld,%ld,%.2f,%.2f\n",
               time_ms,
               force_raw,
               force_filtered,
               angle_deg);

        sleep_ms(50);
    }

    return 0;
}