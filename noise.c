#include "noise.h"

// from http://stackoverflow.com/questions/16746971/what-is-the-fastest-way-to-generate-pseudo-random-number-by-8-bit-mcu

#define STATE_BYTES 7
#define MULT 0x13B /* for STATE_BYTES==6 only */
#define MULT_LO (MULT & 255)
#define MULT_HI (MULT & 256)

char NOISE_calculate(void)
{
    static unsigned char state[STATE_BYTES] =
    { 0x87, 0xdd, 0xdc, 0x10, 0x35, 0xbc, 0x5c };
    static unsigned short c = 0x42;
    static int i = 0;
    unsigned short t;
    unsigned char x;

    x = state[i];
    t = (unsigned short)x * MULT_LO + c;
    c = t >> 8;
#if MULT_HI
    c += x;
#endif
    x = t & 255;
    state[i] = x;
    if (++i >= sizeof(state))
        i = 0;

    return (x>128);
}


