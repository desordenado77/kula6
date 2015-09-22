
#ifndef MIDI_H
#define MIDI_H
#include <stdio.h>

#define VOICES 6
#define NOTE_BUFFER (VOICES*2)
#define CONTROL_CHANGE 128


#define MIDI_NOTE_OFF       0x80 // Note off
#define MIDI_NOTE_ON        0x90 // Note on
#define MIDI_AFTER_TOUCH    0xA0 // After touch
#define MIDI_CONTROL_CHANGE 0xB0 // CC
#define MIDI_PITCH_BEND     0xE0 // Pitch bend
#define MIDI_PROGRAM_CHANGE 0xC0 // Pitch bend
#define MIDI_CHANNEL_CHANGE 0xD0 // Channel change
#define MIDI_SYSTEM_COMMAND 0xF0 // System commands

typedef struct {
    char note;
    char velocity;
} voice_data_t;

int MIDI_sysex_stdout(char c, FILE *stream);

void MIDI_parse(unsigned char data);

void MIDI_init();

#endif // MIDI_H

