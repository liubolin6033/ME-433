#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "pico/stdlib.h"

#define HX711_SCK 14
#define HX711_DT  15

#define ALPHA 0.10f

void hx711_init(void) {
    gpio_init(HX711_SCK);
    gpio_set_dir(HX711_SCK, GPIO_OUT);
    gpio_put(HX711_SCK, 0);

    gpio_init(HX711_DT);
    gpio_set_dir(HX711_DT, GPIO_IN);
}

int32_t hx711_read(void) {
    uint32_t raw = 0;

    while (gpio_get(HX711_DT)) {
        tight_loop_contents();
    }

    for (int i = 0; i < 24; i++) {
        gpio_put(HX711_SCK, 1);
        sleep_us(1);

        raw = raw << 1;
        if (gpio_get(HX711_DT)) {
            raw |= 1;
        }

        gpio_put(HX711_SCK, 0);
        sleep_us(1);
    }

    gpio_put(HX711_SCK, 1);
    sleep_us(1);
    gpio_put(HX711_SCK, 0);
    sleep_us(1);

    if (raw & 0x800000) {
        raw |= 0xFF000000;
    }

    return (int32_t)raw;
}

int main(void) {
    stdio_init_all();
    hx711_init();

    sleep_ms(2000);

    printf("HW14 HX711 ready.\n");
    printf("Send number of samples:\n");

    while (true) {
        int samples = 0;

        int result = scanf("%d", &samples);

        if (result == 1 && samples > 0) {
            printf("Collecting %d samples...\n", samples);
            printf("time_ms,raw,filtered\n");

            absolute_time_t start_time = get_absolute_time();

            int32_t raw = hx711_read();
            float filtered = (float)raw;

            for (int i = 0; i < samples; i++) {
                raw = hx711_read();

                filtered = ALPHA * (float)raw + (1.0f - ALPHA) * filtered;

                int64_t time_ms = absolute_time_diff_us(start_time, get_absolute_time()) / 1000;

                printf("%lld,%ld,%.2f\n", time_ms, raw, filtered);
            }

            printf("END\n");
            printf("Send number of samples:\n");
        }
    }

    return 0;
}