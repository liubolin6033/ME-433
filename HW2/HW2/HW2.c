#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define SERVO_PIN 16

void set_servo_angle(uint pin, float angle) {
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    float pulse_ms = 0.5f + (angle / 180.0f) * 2.0f;
    uint16_t level = (uint16_t)(pulse_ms * 1000.0f);

    pwm_set_gpio_level(pin, level);
}

int main() {
    stdio_init_all();

    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(SERVO_PIN);

    pwm_set_clkdiv(slice_num, 125.0f);   // 1 MHz
    pwm_set_wrap(slice_num, 20000 - 1);  // 20 ms period

    pwm_set_enabled(slice_num, true);

    while (1) {
        for (float angle = 0; angle <= 180; angle += 2) {
            set_servo_angle(SERVO_PIN, angle);
            sleep_ms(20);
        }

        for (float angle = 180; angle >= 0; angle -= 2) {
            set_servo_angle(SERVO_PIN, angle);
            sleep_ms(20);
        }
    }
}