#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
#include "hardware/clocks.h"
#include <stdint.h>

// Musical note frequencies for piano (Hz)
#define NOTE_C4  261.626f
#define NOTE_D4  293.665f
#define NOTE_E4  329.628f
#define NOTE_F4  349.228f
#define NOTE_G4  391.995f
#define NOTE_A4  440.000f
#define NOTE_B4  493.883f

// Arrays to track multiple notes at same time
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

// Global volume (0..65535). Default full scale.
static volatile uint32_t piano_volume = 65535u;

// Forward declarations so other modules (e.g., main.c) can call into here
void init_wavetable(void);
void set_piano_freq(int note_index, float frequency);
void stop_piano_note(int note_index);
float get_note_frequency(int note_index);
void pwm_audio_handler(void);


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

    // Apply global volume (mixed_sample: 0..65535, piano_volume: 0..65535)
    uint32_t scaled = 0;
    if (mixed_sample > 0) {
        uint64_t tmpv = (uint64_t)(uint32_t)mixed_sample * (uint64_t)piano_volume;
        // divide by 65535 to keep 0..65535 range
        scaled = (uint32_t)(tmpv / 65535u);
    }

    // Output to PWM using 16-bit scaling: output = (scaled / 65536) * period
    uint period = pwm_hw->slice[slice_36].top + 1;
    uint64_t tmpp = (uint64_t)scaled * (uint64_t)period;
    uint32_t output = (uint32_t)(tmpp >> 16); // divide by 65536
    if (output >= period) output = period - 1;
    pwm_set_gpio_level(36, (uint16_t)output);
}

// Map a 12-bit ADC value (0..4095) into piano_volume (0..65535)
void set_volume_from_adc(uint16_t adc_value) {
    if (adc_value > 4095u) adc_value = 4095u;
    // scale 0..4095 -> 0..65535
    piano_volume = ((uint32_t)adc_value * 65535u) / 4095u;
}

// Initialize PWM hardware for audio output on the specified GPIO.
// gpio: GPIO number to output PWM on
// top: PWM wrap value (period = top + 1)
// sample_rate: desired IRQ/sample rate (Hz)
void init_pwm_for_audio(uint gpio, uint32_t top, float sample_rate) {
    // Configure GPIO for PWM
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(gpio);
    uint chan = pwm_gpio_to_channel(gpio);

    // Set the PWM period (wrap/top) and start with 0 duty
    pwm_set_wrap(slice, top);
    pwm_set_chan_level(slice, chan, 0);

    // Compute clock divider so wrap frequency ~ sample_rate
    // sample_rate ≈ sys_clock / (clkdiv * (top + 1))
    uint32_t sys_hz = (uint32_t)clock_get_hz(clk_sys);
    float clkdiv = (float)sys_hz / (sample_rate * (float)(top + 1));
    pwm_set_clkdiv(slice, clkdiv);

    // Hook up IRQ handler and enable it
    irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_audio_handler);
    irq_set_enabled(PWM_IRQ_WRAP, true);
    pwm_clear_irq(slice);
    pwm_set_irq_enabled(slice, true);

    // Enable the slice
    pwm_set_enabled(slice, true);
}


