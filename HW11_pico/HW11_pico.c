#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

// UART settings
#define UART_ID uart0
#define BAUD_RATE 115200

// Pico UART pins
#define UART_TX_PIN 0
#define UART_RX_PIN 1

int main()
{
    // USB serial
    stdio_init_all();

    // Initialize UART
    uart_init(UART_ID, BAUD_RATE);

    // Set GPIO functions
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Small startup delay
    sleep_ms(2000);

    printf("Pico UART bridge started\n");

    while (true)
    {
        // STM32 -> Pico USB serial
        if (uart_is_readable(UART_ID))
        {
            char c = uart_getc(UART_ID);

            // Print received character to computer
            printf("%c", c);
        }

        // Computer -> STM32
        int ch = getchar_timeout_us(0);

        if (ch != PICO_ERROR_TIMEOUT)
        {
            uart_putc(UART_ID, (char)ch);
        }
    }
}