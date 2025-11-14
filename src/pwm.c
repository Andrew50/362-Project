#include "support.h"

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

// Function to set PWM frequency for piano notes
//note_index: 0-6 for C-B
//frequency: self explanatory (units: hz)
void set_piano_freq(int note_index, float frequency) {
    // Convert frequency to PWM step value for audio generation
    if (note_index >= 0 && note_index < 7) {
        // PWM step calculation for sine wave generation
        piano_steps[note_index] = (frequency * 1000 / 20000) * 65536;
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
    // Generate 1000-point sine wave lookup table
    // This provides smooth audio output via PWM
    for(int i = 0; i < 1000; i++) {
        wavetable[i] = (16383 * sin(2 * 3.14159 * i / 1000)) + 16384;
    }
}