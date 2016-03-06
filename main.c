#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "RX_UART.h"
#include "BLrxtx.h"
#include "BLmsg.h"

float ScaleValueChange;
uint16_t ScaleValueDetect;
uint8_t ScaleValueDetectChange;

char StrScaleValueDetect[6];
char StrOCR[6];
char DebugAskAnswer[16];

char *StrScaleDetectptr;
char *StrOCRptr;



int main(void)
{
	SW_RX_Init(); 
	BL_Init(MYUBRR);
	PWM_Init();
		
    while (1) 
    {
			SW_GetScaleValue();
			
			if ((ScaleValue > 0) && (ScaleValue != ScaleValueChange))
			{
				ScaleValueChange = ScaleValue;
				StrScaleDetectptr = IntToStrKey(ScaleValueDetect, StrScaleValueDetect, 's');
				StrOCRptr = IntToStrKey(OCR2A, StrOCR, 'o');
				StrPWMValueptr = IntToStrKey(PWMvalue, StrPWMValue, 'p');
				
				BL_SendStr (SWscaleValueForBL);
				BL_SendStr(StrScaleDetectptr);
				BL_SendStr(StrPWMValueptr);
				BL_SendStr(StrOCRptr);
				
				
			}
			SWmesIsComplete = 0;		
						
		if (BLmesIsComplete) 
			{
			//BL_SendStr (BluetoothMessage);
						
			BL_DefComd(); // defining gotten message from bluetooth (smartphone)
			BLmesIsComplete = 0;  // reset flag "complete message from smartphone"
			}
				
		if (PWMvalue && (ScaleValue > 20))
				{
					PWM_PinValue();   // write gotten correction value from smartphone to OCR2A for change OC2A pin PWM
					ScaleValueDetect = ScaleValue; 
				}	
				
		if((ScaleValue < (ScaleValueDetect - 2)) && (ScaleValue > 5))
		{
			OCR2A = 0;
			ScaleValueDetect = 0;
		}
	}
}

