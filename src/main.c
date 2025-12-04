#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
//#include "chardisp.h"
#include "pwm_peter.h"
#include "hardware/adc.h"

//////////////////////////////////////////////////////////////////////////////
// Demo selection: uncomment exactly ONE of these to choose which module runs
//////////////////////////////////////////////////////////////////////////////
// #define AJ
// #define MADDIE
// #define PETER
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

uint16_t vals[7];

void cd_init();
void cd_display1(const char *str);
void cd_display2(const char *str);
void init_chardisp_pins();

// ADC helpers from adc_zac.c
void init_adc(void);
void adc_update_all(void);

// Entry points from teammate modules
int adc_zac_main(void);
void gpio_main(void);
void show_pitch_volume(const char *pitch, uint8_t volume_percent);

// LED matrix display functions from gpio_maddie.c
void update_bar_heights(void);
void display_note(uint8_t row);

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////


/*int main()
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
    gpio_main();
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
    init_adc();

    // Track previous key states to detect press/release
    bool key_was_pressed[7] = {false};
    const uint16_t KEY_THRESHOLD = 2100;

    // Main loop: play notes based on IR sensor keys in vals[0..6]
    int print_timer_ms = 0;
    for (;;) {
        // Update ADC readings into vals[] (reads all channels)
        adc_update_all();

        // Check keys vals[0..6] for press/release (filled by adc_zac)
        for (int key = 0; key < 7; key++) {
            bool is_pressed = (vals[key] > KEY_THRESHOLD);

            // Key pressed (transition from not pressed to pressed)
            if (is_pressed && !key_was_pressed[key]) {
                set_piano_freq(key, get_note_frequency(key)); // note 0..6 (C4..B4)
                key_was_pressed[key] = true;
            }
            // Key released (transition from pressed to not pressed)
            else if (!is_pressed && key_was_pressed[key]) {
                stop_piano_note(key);
                key_was_pressed[key] = false;
            }
        }
        sleep_ms(10);
        print_timer_ms += 10;
        if (print_timer_ms >= 250) {
            // Print PWM debug info similar to ADC printouts
                 // Overwrite the same line like the ADC printout (no new-line)
                 printf("PWM: mixed=%5u scaled=%5u out=%5u active=%u\r",
                     (unsigned)pwm_last_mixed,
                     (unsigned)pwm_last_scaled,
                     (unsigned)pwm_last_output,
                     (unsigned)pwm_last_active);
                 fflush(stdout);
            print_timer_ms = 0;
        }
    }

    return 0;
#endif

    // If no demo is selected, just return.
    return 0;
}*/

int main()
{
    // Initialize all subsystems
    stdio_init_all();
    init_chardisp_pins();
    cd_init();
    gpio_main();                              // Initialize LED matrix GPIOs
    init_wavetable();                         // Initialize PWM wavetable
    init_pwm_for_audio(36, 999, 20000.0f);    // Initialize PWM audio on GPIO 36
    init_adc();                               // Initialize ADC for IR sensors

    // Note names for LCD display (indices 0-6 map to C4-B4)
    const char *note_names[7] = {"C4", "D4", "E4", "F4", "G4", "A4", "B4"};

    // Track previous key states to detect press/release
    bool key_was_pressed[7] = {false};
    int last_pressed_key = -1;
    const uint16_t KEY_THRESHOLD = 2100;

    // Timers for periodic updates
    int print_timer_ms = 0;
    int lcd_timer_ms = 0;

    for (;;) {
        // Update ADC readings into vals[] (reads all 7 IR sensor channels)
        adc_update_all();

        // Check keys vals[0..6] for press/release
        for (int key = 0; key < 7; key++) {
            bool is_pressed = (vals[key] > KEY_THRESHOLD);

            // Key pressed (transition from not pressed to pressed)
            if (is_pressed && !key_was_pressed[key]) {
                set_piano_freq(key, get_note_frequency(key)); // note 0..6 (C4..B4)
                key_was_pressed[key] = true;
                last_pressed_key = key;
            }
            // Key released (transition from pressed to not pressed)
            else if (!is_pressed && key_was_pressed[key]) {
                stop_piano_note(key);
                key_was_pressed[key] = false;
            }
        }

        update_bar_heights();

        for (uint8_t row = 0; row < 16; row++) {
            display_note(row);
        }

        sleep_ms(10);
        print_timer_ms += 10;
        lcd_timer_ms += 10;

        if (print_timer_ms >= 250) {
            printf("PWM: mixed=%5u scaled=%5u out=%5u active=%u\r",
                (unsigned)pwm_last_mixed,
                (unsigned)pwm_last_scaled,
                (unsigned)pwm_last_output,
                (unsigned)pwm_last_active);
            fflush(stdout);
            print_timer_ms = 0;
        }

        if (lcd_timer_ms >= 200) {
            const char *pitch = (pwm_last_active > 0 && last_pressed_key >= 0)
                                ? note_names[last_pressed_key]
                                : "---";

            uint8_t volume_percent = (pwm_last_active > 0)
                                     ? (uint8_t)((pwm_last_scaled * 100) / 65535)
                                     : 0;

            show_pitch_volume(pitch, volume_percent);
            lcd_timer_ms = 0;
        }
    }

    return 0;
}