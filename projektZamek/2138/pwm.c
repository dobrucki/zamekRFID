#include "pwm.h"

void initPWM(tU8 frequency)
{
	PINSEL0 |= 0x00080000;

	PWM_PR = 0x00+60;
	PWM_MCR = 0x02;
	PWM_MR0 = frequency;
	PWM_MR6 = PWM_MR0 / 2;
	PWM_LER = 0x41;
	PWM_PCR = 0x4001;
	PWM_TCR = 0x00;
}

void pwmON() {
    PWM_TCR = 0x09;
}

void pwmOFF(){
    PWM_TCR = 0x02;
}
