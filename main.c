#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "RX_UART.h"
#include "BLrxtx.h"
#include "BLmsg.h"

void WaitSketch()
{
	DDRC |= (1 << PORTC0)|(1 << PORTC1)|(1 << PORTC2)|(1 << PORTC3);
	PORTC |= (1 << PORTC3);
}

int main(void)
{
	SW_RX_Init();
	//BL_Init(MYUBRR);
	BL_Init();
	PWM_Init();
	WaitSketch();
	DDRD |= (1 << PORTD4) | (1 << PORTD5) | (1 << PORTD6) | (1 << PORTD7); // Pin for push Zero Button on Scale
	PORTD |= (1 << PORTD4) |(1 << PORTD5) |(1 << PORTD6) | (1 << PORTD7);

	while (1)
	{
		
					
		SW_GetScaleValue();

		BL_DefComd();

		BL_SetCorrect();

		BL_SendMsg();

		if (DefineScaleMode != 0)
		{
			DefineScale();
		}
	}
}
