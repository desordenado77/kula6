
#ifndef LED_H
#define LED_H

static inline LED_toggle(){
    PORTC ^= (1<<0);
}

static inline LED_turnON(){
    PORTC |= (1<<0);
}

static inline LED_turnOFF(){
    PORTC &= ~(1<<0);
}

#endif // LED_H
