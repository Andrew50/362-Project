
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "chardisp.h"

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

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

static void show_pitch_volume(const char *pitch, uint8_t volume_percent)
{
	char line1[17];
	char line2[17];

	if (volume_percent > 100) {
		volume_percent = 100;
	}

	for (int i = 0; i < 16; i++) line1[i] = ' ';
	line1[16] = '\0';
	line1[0] = 'P'; line1[1] = ':'; line1[2] = ' ';
	if (pitch) {
		for (int i = 0; i < 13 && pitch[i] != '\0'; i++) {
			line1[3 + i] = pitch[i];
		}
	}

	for (int i = 0; i < 16; i++) line2[i] = ' ';
	line2[16] = '\0';
	{
		char buf[16];
		int n = snprintf(buf, sizeof(buf), "V: %u%%", (unsigned)volume_percent);
		if (n < 0) n = 0;
		if (n > 16) n = 16;
		for (int i = 0; i < n && i < 16; i++) {
			line2[i] = buf[i];
		}
	}

	cd_display1(line1);
	cd_display2(line2);
}

int main()
{
	stdio_init_all();
	init_chardisp_pins();
	cd_init();

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
}


void pwm_audio_handler() {
    // PWM interrupt handler for multi-note piano audio generation
    uint slice_36 = pwm_gpio_to_slice_num(36);
    pwm_clear_irq(slice_36);
    
    int mixed_sample = 0;
    int active_count = 0;
    
    // Mix all active piano notes
    for (int i = 0; i < 7; i++) {
        if (piano_active[i]) {
            // Advance position for this note
            piano_positions[i] += piano_steps[i];
            
            // Wrap around at end of wavetable
            if (piano_positions[i] >= (N << 16)) {
                piano_positions[i] -= (N << 16);
            }
            
            // Get sample from wavetable
            int sample_index = piano_positions[i] >> 16;
            mixed_sample += wavetable[sample_index];
            active_count++;
        }
    }
    
    // Average the mixed samples to prevent clipping
    if (active_count > 0) {
        mixed_sample = mixed_sample / active_count;
    }
    
    // Output to PWM
    uint period = pwm_hw->slice[slice_36].top + 1;
    int output = (mixed_sample * period) / (1 << 16);
    pwm_set_gpio_level(36, output);
}