#ifndef PWM_PETER_H
#define PWM_PETER_H

#include <stdbool.h>
#include <stdint.h>

// Global ADC values (updated by adc_zac module)
extern uint16_t vals[7];

// Expose piano control API for use from main.c or other modules
void init_wavetable(void);
void init_pwm_for_audio(unsigned gpio, unsigned int top, float sample_rate);
void set_volume_from_adc(uint16_t adc_value);
void set_piano_freq(int note_index, float frequency);
void stop_piano_note(int note_index);
float get_note_frequency(int note_index);
void pwm_audio_handler(void);

// Optional: expose state arrays if other modules need direct access
extern int piano_steps[7];
extern int piano_positions[7];
extern bool piano_active[7];

#endif


