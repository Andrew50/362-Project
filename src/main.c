#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
#include "queue.h"
#include "support.h"

//////////////////////////////////////////////////////////////////////////////

static int duty_cycle = 0;
static int dir = 0;
static int color = 0;

void display_init_pins();
void display_init_timer();
void display_char_print(const char message[]);
void keypad_init_pins();
void keypad_init_timer();
void init_wavetable(void);
void set_piano_freq(int note_index, float frequency);
void stop_piano_note(int note_index);
float get_note_frequency(int note_index);
extern KeyEvents kev;

///////////////////////////////////////////////////////////

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