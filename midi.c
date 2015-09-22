

#include <avr/io.h>
#include "midi.h"



volatile char channel_mask = 0xff;
volatile short pitch_bend = 0;
volatile voice_data_t voices[NOTE_BUFFER];
volatile char control[CONTROL_CHANGE];

static void process_midi(char * data, char size);

static void uart_putc(unsigned char c)
{
    // wait until UDR ready
    while(!(UCSRA & (1 << UDRE)));
    UDR = c;    // send character
}

/* This function is used to redirect stdout as sysex command
*/
int MIDI_sysex_stdout(char c, FILE *stream){
    uart_putc(0xf0);
    uart_putc(0x41);
    uart_putc(c);
    uart_putc(0xf7);
}


void MIDI_parse(unsigned char data){
    static char stage = 0;
    static unsigned char operation;
    static char size = 0;
    static char param[2];
    static char running_status = 0;
    static char current_midi_data[5];
    static char current_midi_size;

    if((stage != 0) && (data & 0x80)){
        stage = 0;
    }

    if((stage == 0) && ((data & 0x80) == 0) && (running_status != 0)){
        operation = running_status & 0xF0;
        switch (operation)
        {
            case MIDI_NOTE_OFF: // Note off
            case MIDI_NOTE_ON: // Note on
            case MIDI_AFTER_TOUCH: // After touch
            case MIDI_CONTROL_CHANGE: // CC
            case MIDI_PITCH_BEND: // Pitch bend
                size = 2;
                stage = 1;
                break;

            case MIDI_PROGRAM_CHANGE: // Program change
                size = 1;
                stage = 1;
                break;
        }
        current_midi_data[0] = running_status;
        current_midi_size = size+1;
    }

    if(stage == 0){
        operation = data & 0xF0;
        running_status = 0;

        switch (operation)
        {
            case MIDI_NOTE_OFF: // Note off
            case MIDI_NOTE_ON: // Note on
            case MIDI_AFTER_TOUCH: // After touch
            case MIDI_CONTROL_CHANGE: // CC
            case MIDI_PITCH_BEND: // Pitch bend
                size = 2;
                stage = 1;
                running_status = data;
                break;

            case MIDI_PROGRAM_CHANGE: // Program change
                size = 1;
                stage = 1;
                running_status = data;
                break;

            case 0xD0: // Channel change
                size = 1;
                stage = 1;
                break;

            case 0xF0: // System commands
                if(data == 0xF0){
                    // sysex
                    size = -1;
                    stage = 0;
                }
                else {
                    // size 0 to 2
                    if(data == 0xF1){
                        size = 1;
                        stage = 1;
                    } else if(data == 0xF2){
                        size = 2;
                        stage = 1;
                    } else if(data == 0xF3){
                        size = 1;
                        stage = 1;
                    } else if(data == 0xF4){
                        // ignore
                        size = -1;
                        stage = 0;
                    } else if(data == 0xF5){
                        // ignore
                        size = -1;
                        stage = 0;
                    } else if(data == 0xF6){
                        // size 0
                        size = 0;
                        stage = 0;
                    } else if(data == 0xF7){
                        // size 0
                        size = 0;
                        stage = 0;
                    } else if(data >= 0xF8 && data <= 0xFF){
                        // size 0
                        size = 0;
                        stage = 0;
                    }
                }
                break;

            default:
                stage = 0;
                break;
        }
        current_midi_data[0] = data;
        current_midi_size = size+1;
    }
    else {
        current_midi_data[stage] = data;
        stage++;
    }

    if(stage>size) {
        stage = 0;

        if(size != -1){
            //ready
            process_midi(&current_midi_data[0], current_midi_size);
//            printf("--- received %02x %02x %02x\n", current_midi_data[0], current_midi_data[1], current_midi_data[2]);
        }
    }


    return;
}



/*
void send_note_on(){
    uart_putc(144);
    uart_putc(64);
    uart_putc(90);
}

void send_note_off(){
    uart_putc(128);
    uart_putc(64);
    uart_putc(40);
}
*/

static void set_midi_note(char note, char velocity){
    char i, j;
    char found = 0;
//printf("-- note %d %d\n", note, velocity);
    for(i=0;i<NOTE_BUFFER;i++){
        if(voices[i].note == note) {
            voices[i].velocity = velocity;
            if(i<VOICES) {
                if(velocity == 0) {
                    for(j=VOICES;j<NOTE_BUFFER;j++){
                        if(voices[j].velocity != 0) {
                            voices[i].note = voices[j].note;
                            voices[i].velocity = voices[j].velocity;
                            voices[j].velocity = 0;
                            found = 1;
                            break;
                        }
                    }
                }
            }
            else {
                if(velocity != 0) {
                    for(j=0;j<VOICES;j++){
                        if(voices[j].velocity == 0) {
                            voices[j].note = note;
                            voices[j].velocity = velocity;
                            voices[i].velocity = 0;
                            found = 1;
                            break;
                        }
                    }
                }
            }
            found = 1;
            break;
        }
    }

    if(found == 0) {
        // Note not found add to first empty slot
        for(j=0;j<NOTE_BUFFER;j++){
            if(voices[j].velocity == 0) {
                voices[j].note = note;
                voices[j].velocity = velocity;
                break;
            }
        }
    }
}

void MIDI_init() {
    int j;

    for(j=0;j<NOTE_BUFFER;j++){
        voices[j].note = 255;
        voices[j].velocity = 0;
    }
}

void MIDI_show() {
    char j;
    printf("---------------------\n");
    for(j=0;j<NOTE_BUFFER;j++){
        if(voices[j].velocity)
            printf("%d) note %d vel %d\n", j, voices[j].note, voices[j].velocity);
    }
}

static void process_midi(char * data, char size) {
    unsigned char operation = data[0] & 0xF0;
    unsigned char channel = data[0] & 0x0F;

    if((1<<channel) & channel_mask) {
        switch(operation) {
        case MIDI_NOTE_ON:
            set_midi_note(data[1], data[2]);
            break;
        case MIDI_NOTE_OFF:
            set_midi_note(data[1], 0);
            break;

        }
    }
}

