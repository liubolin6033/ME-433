#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "ssd1306.h"
#include "font.h"

#define SDA_PIN 4
#define SCL_PIN 5
#define LED_PIN 16
#define ADC_PIN 26

void drawChar(int x, int y, char c) {
    if (c < 0x20 || c > 0x7E) {
        return;
    }

    int index = c - 0x20;

    for (int col = 0; col < 5; col++) {
        char line = ASCII[index][col];

        for (int row = 0; row < 8; row++) {
            int pixel_on = (line >> row) & 0x01;
            ssd1306_drawPixel(x + col, y + row, pixel_on);
        }
    }
}

void drawMessage(int x, int y, char *message) {
    int i = 0;
    while (message[i] != '\0') {
        drawChar(x + i * 6, y, message[i]);
        i++;
    }
}

int main() {
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    i2c_init(i2c_default, 400000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(0);

    sleep_ms(250);
    ssd1306_setup();

    bool on = false;
    unsigned int t_last = to_us_since_boot(get_absolute_time());

    char msg1[50];
    char msg2[50];

    while (true) {
        unsigned int t_now = to_us_since_boot(get_absolute_time());
        float fps = 1000000.0f / (t_now - t_last);
        t_last = t_now;

        uint16_t raw = adc_read();
        float voltage = raw * 3.3f / 4095.0f;

        sprintf(msg1, "ADC0=%.2fV", voltage);
        sprintf(msg2, "FPS=%.1f", fps);

        on = !on;
        gpio_put(LED_PIN, on);

        ssd1306_clear();
        drawMessage(0, 0, msg1);
        drawMessage(0, 24, msg2);
        ssd1306_drawPixel(120, 0, on);
        ssd1306_update();

        sleep_ms(50);
    }
}