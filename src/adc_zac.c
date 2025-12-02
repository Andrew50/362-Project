#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

// Global ADC values array
uint16_t adc_vals[7] = {0};

void init_adc(void) {
    stdio_init_all();
    adc_init();

    // Enable the GPIOs that correspond to ADC channels 0–6
    adc_gpio_init(40);
    adc_gpio_init(41);
    adc_gpio_init(42);
    adc_gpio_init(43);
    adc_gpio_init(44);
    adc_gpio_init(45);
    adc_gpio_init(46);
}

static uint16_t read_adc_channel(uint ch) {
    adc_select_input(ch);   // pick channel
    sleep_us(5);            // tiny settling time (being safe)
    return adc_read();      // blocking read of a single sample
}

int adc_zac_main(void) {
    init_adc();

    while (true) {
        // Take one sample per channel and store in adc_vals
        for (int ch = 0; ch < 7; ch++) {
            adc_vals[ch] = read_adc_channel(ch);
        }

        // Print them all on one line
        printf("ADC: ");
        for (int ch = 0; ch < 7; ch++) {
            printf("CH%d=%4u  ", ch, adc_vals[ch]);
        }
        printf("\r");       // overwrite same line
        fflush(stdout);

        sleep_ms(250);
    }

    return 0;
}