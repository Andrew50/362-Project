#include <stdio.h>
#include "pico/stdlib.h"

int main() {
    stdio_init_all();

    // ABCD is the select row
   
    const int PIN_1 = 25; // R1
    const int PIN_2 = 26; // B1
    const int PIN_3 = 27; // G1
    const int PIN_4 = 27; // R2
    const int PIN_5 = 28; // B2
    const int PIN_6 = 29; // G2
    const int PIN_7 = 30; // clk
    const int PIN_8 = 42; // oe
    const int PIN_9 = 43; // lat
    const int PIN_10 = 20; // A
    const int PIN_11 = 21; // B
    const int PIN_12 = 22; // C
    const int PIN_13 = 23; // D

    // INITIALIZE ALL PINS
    gpio_init(PIN_1);
    gpio_init(PIN_2);
    gpio_init(PIN_3);
    gpio_init(PIN_4);
    gpio_init(PIN_5);
    gpio_init(PIN_6);
    gpio_init(PIN_7);
    gpio_init(PIN_8);
    gpio_init(PIN_9);
    gpio_init(PIN_10);
    gpio_init(PIN_11);
    gpio_init(PIN_12);
    gpio_init(PIN_13);

    // SET ALL GPIO PINS AT OUTPUTS
    gpio_set_dir(PIN_1, GPIO_OUT);
    gpio_set_dir(PIN_2, GPIO_OUT);
    gpio_set_dir(PIN_3 , GPIO_OUT);
    gpio_set_dir(PIN_4 , GPIO_OUT);
    gpio_set_dir(PIN_5, GPIO_OUT);
    gpio_set_dir(PIN_6 , GPIO_OUT);
    gpio_set_dir(PIN_7 , GPIO_OUT);
    gpio_set_dir(PIN_8 , GPIO_OUT);
    gpio_set_dir(PIN_9 , GPIO_OUT);
    gpio_set_dir(PIN_10 , GPIO_OUT);
    gpio_set_dir(PIN_11 , GPIO_OUT);
    gpio_set_dir(PIN_12 , GPIO_OUT);
    gpio_set_dir(PIN_13 , GPIO_OUT);

    // Enable
    gpio_put(PIN_8, 1);

    // Row select
    gpio_put(PIN_10, 1);
    gpio_put(PIN_11, 0);
    gpio_put(PIN_12, 0);
    gpio_put(PIN_13, 0);

    // clk and lat ???
    gpio_put(PIN_8, 1);
    gpio_put(PIN_9, 1);

    // Send different colors to the LED Matrix
    gpio_put(PIN_7, 0);
    gpio_put(PIN_8, 0);
    gpio_put(PIN_9, 0);
    gpio_put(PIN_10, 0);

    gpio_put(PIN_1, 1);
    gpio_put(PIN_2, 0);
    gpio_put(PIN_3, 0);

    sleep_ms(500);

    gpio_put(PIN_1, 0);
    gpio_put(PIN_2, 1);
    gpio_put(PIN_3, 0);

    sleep_ms(500);

    gpio_put(PIN_1, 0);
    gpio_put(PIN_2, 0);
    gpio_put(PIN_3, 1);

    sleep_ms(500);

    gpio_put(PIN_4, 1);
    gpio_put(PIN_5, 0);
    gpio_put(PIN_6, 0);

    sleep_ms(500);

    gpio_put(PIN_4, 0);
    gpio_put(PIN_5, 1);
    gpio_put(PIN_6, 0);

    sleep_ms(500);

    gpio_put(PIN_4, 0);
    gpio_put(PIN_5, 0);
    gpio_put(PIN_6, 1);

    sleep_ms(500);
    
}