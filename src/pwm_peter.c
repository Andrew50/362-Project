#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
#include "queue.h"
#include "support.h"
#include <stdint.h>

//////////////////////////////////////////////////////////////////////////////
// Combined PWM / piano audio module
//  - Note frequency helpers (from pwm.c)
//  - Wavetable initialization (from pwm.c)
//  - PWM interrupt audio handler (from peter_main.c)
//////////////////////////////////////////////////////////////////////////////

// Musical note frequencies for piano (Hz)
#define NOTE_C4  261.626f
#define NOTE_D4  293.665f
#define NOTE_E4  329.628f
#define NOTE_F4  349.228f
#define NOTE_G4  391.995f
#define NOTE_A4  440.000f
#define NOTE_B4  493.883f

// Arrays to track multiple notes simultaneously
int piano_steps[7] = {0};        // PWM step value for each note
int piano_positions[7] = {0};    // Current playback position for each note
bool piano_active[7] = {false};  // Whether each note is currently playing

// Lookup table for note frequencies
static const float note_freq_table[7] = {
    NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_A4, NOTE_B4
};

// Variables carried over from peter_main.c (kept in case other code uses them)
static int duty_cycle = 0;
static int dir = 0;
static int color = 0;

// Wavetable length (number of samples in the lookup table)
#define N 1000

// Wavetable used for audio synthesis (values scaled 0..65535)
static uint32_t wavetable[N];

// Forward declarations so other modules (e.g., main.c) can call into here
void init_wavetable(void);
void set_piano_freq(int note_index, float frequency);
void stop_piano_note(int note_index);
float get_note_frequency(int note_index);
void pwm_audio_handler(void);

//////////////////////////////////////////////////////////////////////////////
// Piano note helpers (from pwm.c)
//////////////////////////////////////////////////////////////////////////////

// Set PWM step value for a given piano note based on frequency
// note_index: 0-6 for C-B
// frequency: frequency in Hz
void set_piano_freq(int note_index, float frequency) {
    // Convert frequency to PWM step value for audio generation
    if (note_index >= 0 && note_index < 7) {
        // PWM step calculation for sine wave generation
        piano_steps[note_index] = (int)((frequency * 1000.0f / 20000.0f) * 65536.0f);
        piano_active[note_index] = true;  // Mark note as active
    }
}

// Stop a specific piano note
void stop_piano_note(int note_index) {
    if (note_index >= 0 && note_index < 7) {
        piano_active[note_index] = false;
        piano_positions[note_index] = 0;
    }
}

// Get frequency for a note index
float get_note_frequency(int note_index) {
    if (note_index >= 0 && note_index < 7) {
        return note_freq_table[note_index];
    }
    return 0.0f;
}

// Initialize sine wave table for smooth audio (automatically calculate sine values)
void init_wavetable(void) {
    // Generate N-point sine wave lookup table (values scaled 0..65535)
    // This provides smooth audio output via PWM
    for (int i = 0; i < N; i++) {
        float s = sinf(2.0f * 3.14159265358979323846f * (float)i / (float)N);
        // scale from [-1,1] -> [0,65535]
        wavetable[i] = (uint32_t)((s + 1.0f) * 32767.5f + 0.5f);
    }
}

//////////////////////////////////////////////////////////////////////////////
// PWM interrupt handler (from peter_main.c)
//////////////////////////////////////////////////////////////////////////////

// PWM interrupt handler for multi-note piano audio generation
void pwm_audio_handler(void) {
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
    pwm_set_gpio_level(36, (uint16_t)output);
}


