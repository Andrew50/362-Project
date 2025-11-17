#ifndef PWM_PETER_H
#define PWM_PETER_H

#include <stdbool.h>

// Expose piano control API for use from main.c or other modules
void init_wavetable(void);
void set_piano_freq(int note_index, float frequency);
void stop_piano_note(int note_index);
float get_note_frequency(int note_index);
void pwm_audio_handler(void);

// Optional: expose state arrays if other modules need direct access
extern int piano_steps[7];
extern int piano_positions[7];
extern bool piano_active[7];

#endif


