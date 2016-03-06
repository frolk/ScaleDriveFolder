
#include <stdlib.h>
#include <avr/io.h>
#include "BLmsg.h"
#include "BLrxtx.h"
#include <avr/interrupt.h>



//uint32_t dutyCycle = 0;

uint8_t BluetoothMessage[10];
uint16_t PWMvalue = 0;

char *StrPWMValueptr;
char StrPWMValue[6];


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
}



char* IntToStr(uint16_t n, char *buffer)
{
	uint8_t d4, d3, d2, d1, q, d0;

	d1 = (n>>4)  & 0xF;
	d2 = (n>>8)  & 0xF;
	d3 = (n>>12) & 0xF;

	d0 = 6*(d3 + d2 + d1) + (n & 0xF);
	q = (d0 * 0xCD) >> 11;
	d0 = d0 - 10*q;

	d1 = q + 9*d3 + 5*d2 + d1;
	q = (d1 * 0xCD) >> 11;
	d1 = d1 - 10*q;

	d2 = q + 2*d2;
	q = (d2 * 0x1A) >> 8;
	d2 = d2 - 10*q;

	d3 = q + 4*d3;
	d4 = (d3 * 0x1A) >> 8;
	d3 = d3 - 10*d4;

	char *ptr = buffer;
	*ptr++ = ( d4 + '0' );
	*ptr++ = ( d3 + '0' );
	*ptr++ = ( d2 + '0' );
	*ptr++ = ( d1 + '0' );
	*ptr++ = ( d0 + '0' );
	*ptr = 0;

	while(buffer[0] == '0') ++buffer;
	return buffer;
}

char* IntToStrKey(uint16_t val, char *buffer, char key)
{
	char *str;
	str = IntToStr (val, buffer) - 1;
	*str = key;
}



void BL_GetMessage() // getting value from ring buffer to BlutoothMessage array
{
	
	for (int i=0; i<8; i++)
	{
		BluetoothMessage[i] = BL_GetChar();
	}
	BL_FlushRxBuf();  // flush our buffer and start from the beginning

}

void BL_DefComd()
{
	
	BL_GetMessage(); //pulling up buffer's data one by one
	if ((BluetoothMessage[0] == '-')|(BluetoothMessage[0] == '+'))
	{
		PWMvalue = atoi(BluetoothMessage+1);
		
		 //convert our string into float integer
		//BL_FlushRxBuf();
	} 
}

