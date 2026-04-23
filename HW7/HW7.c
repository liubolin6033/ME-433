#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

#define PI 3.1415926535

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

void write_dac(int channel, unsigned short val) {
    if (val > 1023) val = 1023;

    uint16_t command = 0;
    uint8_t data[2];

    command = (channel << 15) | (1 << 13) | (1 << 12) | (val << 2);

    data[0] = (command >> 8) & 0xFF;
    data[1] = command & 0xFF;

    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, data, 2);
    cs_deselect(PIN_CS);
}

int main()
{
    stdio_init_all();

    spi_init(SPI_PORT, 1000 * 1000);

    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    sleep_ms(100);

    int i_sin = 0;
    int i_tri = 0;

    while (1) {

        int val_sin = (int)(511.5 + 511.5 * sin(2.0 * PI * i_sin / 100.0));
        write_dac(0, val_sin);

        i_sin++;
        if (i_sin >= 100) i_sin = 0;

        int val_tri;
        if (i_tri < 100) {
            val_tri = i_tri * 1023 / 99;
        } else {
            val_tri = (199 - i_tri) * 1023 / 99;
        }
        write_dac(1, val_tri);

        i_tri++;
        if (i_tri >= 200) i_tri = 0;

        sleep_ms(5);
    }
}