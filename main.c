#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "RX_UART.h"
#include "BLrxtx.h"
#include "BLmsg.h"


int main(void)
{
	SW_RX_Init(); 
	BL_Init(MYUBRR);
	PWM_Init();
		
    while (1) 
    {
			SW_GetScaleValue();
			
			BL_DefComd();
			
			BL_SetCorrect();	
			
			BL_SendMsg();
			
			if(DefineScaleMode != 0)
			{
				DefineScale();
			}
				
	}
}

