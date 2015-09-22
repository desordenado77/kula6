/*  UART implementation using the ATMEL ATMega32 MCU
    By - Nandan Banerjee (08/CSE/15)
         NIT Durgapur
         16/06/2010
*/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "midi.h"
#include "led.h"

unsigned char data[256];
int readPtr = 0;
int writePtr = 0;
int elements = 0;

#define BAUDRATE 31250

// UART Baud rate - 115200 bps
//#define UBRR 5

void adc_init()
{
 // Select Vref=AVcc
 ADMUX |= (1<<REFS0);
 //set prescaller to 128 and enable ADC
 ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADEN);
}


uint16_t adc_read(uint8_t ADCchannel)
{
 //select ADC channel with safety mask
 ADMUX = (ADMUX & 0xF0) | (ADCchannel & 0x0F);
 //single conversion mode
 ADCSRA |= (1<<ADSC);
 // wait until ADC conversion is complete
 while( ADCSRA & (1<<ADSC) );
 return ADC;
}


void uart_init(int baudrate)
{
 	// calculate division factor for requested baud rate, and set it
	unsigned short bauddiv = ((F_CPU+(baudrate*8L))/(baudrate*16L)-1);
	UBRRL = bauddiv;
#ifdef UBRRH
	UBRRH |= bauddiv>>8;
#endif

    UCSRB = ((1<<RXEN) | (1<<TXEN) | (1<<RXCIE));   // Enable Receiver, Transmitter, Receive Interrupt
    UCSRC = ((1<<URSEL) | (1<<UCSZ1) | (1<<UCSZ0));     // 8N1 data frame
}

ISR(USART_RXC_vect)
{
    unsigned char data = UDR;

    MIDI_parse(data);
}


int main(void)
{
//    midi_data m_data;
//    init_queue(&midi_q);

    adc_init();
    MIDI_init();

    //Setup the clock
    cli();            //Disable global interrupts
    uart_init(BAUDRATE);
    sei();            //Enable global interrupts

    fdevopen(MIDI_sysex_stdout, NULL);

    printf("Hello World\n");

    //Setup the I/O for the LED

    DDRC |= (1<<0);     //Set PortC Pin0 as an output
    PORTC |= (1<<0);        //Set PortC Pin0 high to turn on LED

        _delay_ms(500);
        PORTC ^= (1<<0);
        _delay_ms(500);
        PORTC ^= (1<<0);
        _delay_ms(500);
        PORTC ^= (1<<0);
    int i = 0;
    while(1) {
        i++;
        uint16_t adc_result0 = adc_read(0);      // read adc value at PA0

        printf("-- i %d  adc: %d\n", i, adc_result0);

        MIDI_show();
        _delay_ms(1500);
    }

    return 0;
}
