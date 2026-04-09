#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define I2C_PORT i2c0
#define I2C_SDA 4
#define I2C_SCL 5

#define MCP23008_ADDR 0x20
#define IODIR_REG 0x00
#define GPIO_REG  0x09
#define OLAT_REG  0x0A

#define HEARTBEAT_PIN 16

void write_register(uint8_t reg, uint8_t value) {
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = value;
    i2c_write_blocking(I2C_PORT, MCP23008_ADDR, buf, 2, false);
}

uint8_t read_register(uint8_t reg) {
    uint8_t value;
    i2c_write_blocking(I2C_PORT, MCP23008_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, MCP23008_ADDR, &value, 1, false);
    return value;
}

int main() {
    stdio_init_all();

    gpio_init(HEARTBEAT_PIN);
    gpio_set_dir(HEARTBEAT_PIN, GPIO_OUT);

    i2c_init(I2C_PORT, 100 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    sleep_ms(100);

    write_register(IODIR_REG, 0x7F);

    write_register(OLAT_REG, 0x00);

    bool heartbeat = false;

    while (true) {
        heartbeat = !heartbeat;
        gpio_put(HEARTBEAT_PIN, heartbeat);

        uint8_t gpio_val = read_register(GPIO_REG);

        if ((gpio_val & 0x01) == 0) {
            write_register(OLAT_REG, 0x80);
        } else {
            write_register(OLAT_REG, 0x00);
        }

        sleep_ms(200);
    }
}