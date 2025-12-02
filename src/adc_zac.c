#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

// Global ADC values array (shared with other modules)
uint16_t vals[7] = {0};

void init_adc(void) {
    adc_init();

    adc_gpio_init(40);
    adc_gpio_init(41);
    adc_gpio_init(42);
    adc_gpio_init(43);
    adc_gpio_init(44);
    adc_gpio_init(45);
    adc_gpio_init(46);
}

static uint16_t read_adc_channel(uint ch) {
    adc_select_input(ch);
    sleep_us(5);
    return adc_read();
}

void adc_update_all(void) {
    for (uint ch = 0; ch < 7; ch++) {
        vals[ch] = read_adc_channel(ch);
    }
}

int adc_zac_main(void)
{
    stdio_init_all();
    init_adc();

    while (true) {
        adc_update_all();

        printf("ADC: ");
        for (uint ch = 0; ch < 7; ch++) {
            printf("CH%u=%4u  ", ch, vals[ch]);
        }
        printf("\r");
        fflush(stdout);

        sleep_ms(250);
    }

    return 0;
}