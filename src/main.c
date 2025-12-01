#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "chardisp.h"
#include "pwm_peter.h"
#include "hardware/adc.h"

//////////////////////////////////////////////////////////////////////////////
// Demo selection: uncomment exactly ONE of these to choose which module runs
//////////////////////////////////////////////////////////////////////////////
// #define AJ
// #define PETER
// #define MADDIE
// #define ZAC

//////////////////////////////////////////////////////////////////////////////

const char* username = "Andrew50";

const bool USING_LCD = true;
const int SEG7_DMA_CHANNEL = 0;

const int SPI_7SEG_SCK = 10;
const int SPI_7SEG_CSn = 13;
const int SPI_7SEG_TX = 11;

const int SPI_DISP_SCK = 18;
const int SPI_DISP_CSn = 17;
const int SPI_DISP_TX = 19;

//////////////////////////////////////////////////////////////////////////////

void cd_init();
void cd_display1(const char *str);
void cd_display2(const char *str);
void init_chardisp_pins();

// Entry points from teammate modules
int adc_zac_main(void);
int gpio_maddie_main(void);
void show_pitch_volume(const char *pitch, uint8_t volume_percent);

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////


int main()
{
    stdio_init_all();
    init_chardisp_pins();
    cd_init();

#ifdef ZAC
    // Zac's ADC demo (runs forever)
    return adc_zac_main();
#endif

#ifdef MADDIE
    // Maddie's GPIO / LED matrix demo
    return gpio_maddie_main();
#endif

#ifdef AJ
    // AJ's LCD pitch/volume demo
    const char *test_pitches[] = { "C4", "D4", "E4", "F4", "G4", "A4", "B4", "C5" };
    const uint8_t test_volumes[] = { 10, 25, 40, 55, 70, 85, 100, 0 };
    const int num_tests = (int)(sizeof(test_pitches) / sizeof(test_pitches[0]));
    int idx = 0;

    for (;;) {
        show_pitch_volume(test_pitches[idx], test_volumes[idx]);
        idx = (idx + 1) % num_tests;
        sleep_ms(1000);
    }

    return 0;
#endif

#ifdef PETER

    init_wavetable();

    init_pwm_for_audio(36, 999, 20000.0f);
   
    adc_init();
    adc_gpio_init(26); // ADC0 on GPIO26

    // Play a test note and update volume in a loop
    set_piano_freq(0, get_note_frequency(0)); // start C4
    for (;;) {
        adc_select_input(0);
        sleep_us(5);
        uint16_t v = adc_read();
        set_volume_from_adc(v);
        sleep_ms(10);
    }

    return 0;
#endif

    // If no demo is selected, just return.
    return 0;
}