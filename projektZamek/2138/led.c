#include "led.h"


void initLED() {
    
    PINSEL0 |= (0 << 21) | (0 << 20);
    IODIR0 |= (1 << 10); 
    
}

void ledON() {
    IOSET0 = (1 << 10);
    
}

void ledOFF() {
    IOCLR0 = (1 << 10);
    
}
