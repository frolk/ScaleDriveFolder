
#include <avr/io.h>
#include "BLmsg.h"


void PWM_Init()
{
	DDRB = (1 << PORTB3)|(1 << PORTB5);// set up pin portb3 like output 
	TCCR2A = (1 << WGM21)|(1 << WGM20)|(1<< COM2A1);
	TCCR2B = (1<<CS20);
	OCR2A = 0x00;
}


void PWM_PinValue()
{
	OCR2A = PWMvalue;		
	//PWMvalue = 0;
	
}
