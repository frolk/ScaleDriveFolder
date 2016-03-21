#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "RX_UART.h"
#include "BLrxtx.h"
#include "BLmsg.h"

void WaitSketch()
{
	DDRC |= (1 << PORTC2);
	PORTC |= (1 << PORTC2);
}

int main(void)
{
	SW_RX_Init();
	BL_Init(MYUBRR);
	PWM_Init();
	WaitSketch();

	
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

		//BL_SendStr(SWscaleValueForBL);
		//BL_SendStr("\n\r");

	}
}
