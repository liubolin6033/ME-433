#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

#define SPI_PORT spi0

#define PIN_MISO 16
#define PIN_DAC_CS 17
#define PIN_SCK  18
#define PIN_MOSI 19
#define PIN_RAM_CS 13

#define PI 3.1415926535
#define NUM_SAMPLES 1000

#define RAM_WRITE 0x02
#define RAM_READ  0x03
#define RAM_MODE  0x01
#define RAM_SEQ   0x40

static inline void cs_select(uint cs_pin) {
    asm volatile("nop \n nop \n nop");
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop \n nop");
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop");
}

void dac_write_bytes(uint8_t high, uint8_t low) {
    uint8_t data[2];
    data[0] = high;
    data[1] = low;

    cs_select(PIN_DAC_CS);
    spi_write_blocking(SPI_PORT, data, 2);
    cs_deselect(PIN_DAC_CS);
}

uint16_t make_dac_command(unsigned short val) {
    if (val > 1023) {
        val = 1023;
    }

    uint16_t command;
    command = (0 << 15) | (1 << 13) | (1 << 12) | (val << 2);

    return command;
}

void spi_ram_init() {
    uint8_t data[2];
    data[0] = RAM_MODE;
    data[1] = RAM_SEQ;

    cs_select(PIN_RAM_CS);
    spi_write_blocking(SPI_PORT, data, 2);
    cs_deselect(PIN_RAM_CS);
}

void ram_write_byte(uint16_t address, uint8_t value) {
    uint8_t data[4];

    data[0] = RAM_WRITE;
    data[1] = (address >> 8) & 0xFF;
    data[2] = address & 0xFF;
    data[3] = value;

    cs_select(PIN_RAM_CS);
    spi_write_blocking(SPI_PORT, data, 4);
    cs_deselect(PIN_RAM_CS);
}

void ram_read_2bytes(uint16_t address, uint8_t *high, uint8_t *low) {
    uint8_t tx[5];
    uint8_t rx[5];

    tx[0] = RAM_READ;
    tx[1] = (address >> 8) & 0xFF;
    tx[2] = address & 0xFF;
    tx[3] = 0x00;
    tx[4] = 0x00;

    cs_select(PIN_RAM_CS);
    spi_write_read_blocking(SPI_PORT, tx, rx, 5);
    cs_deselect(PIN_RAM_CS);

    *high = rx[3];
    *low  = rx[4];
}

int main() {
    stdio_init_all();

    spi_init(SPI_PORT, 1000 * 1000);

    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(PIN_DAC_CS);
    gpio_set_dir(PIN_DAC_CS, GPIO_OUT);
    gpio_put(PIN_DAC_CS, 1);

    gpio_init(PIN_RAM_CS);
    gpio_set_dir(PIN_RAM_CS, GPIO_OUT);
    gpio_put(PIN_RAM_CS, 1);

    sleep_ms(100);

    spi_ram_init();

    for (int i = 0; i < NUM_SAMPLES; i++) {
        float angle = 2.0f * PI * i / NUM_SAMPLES;
        float voltage_ratio = 0.5f + 0.5f * sinf(angle);

        unsigned short dac_val = voltage_ratio * 1023;
        uint16_t command = make_dac_command(dac_val);

        uint8_t high_byte = (command >> 8) & 0xFF;
        uint8_t low_byte = command & 0xFF;

        uint16_t address = i * 2;

        ram_write_byte(address, high_byte);
        ram_write_byte(address + 1, low_byte);
    }

    while (1) {
        for (int i = 0; i < NUM_SAMPLES; i++) {
            uint16_t address = i * 2;

            uint8_t high_byte;
            uint8_t low_byte;

            ram_read_2bytes(address, &high_byte, &low_byte);

            dac_write_bytes(high_byte, low_byte);

            sleep_ms(1);
        }
    }
}