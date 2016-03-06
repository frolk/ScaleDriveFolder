#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "RX_UART.h"
#include "BLrxtx.h"
#include "BLmsg.h"
#include "PWM.h"

float ScaleValue;
float ScaleValueChange;
uint16_t ScaleValueDetect;
uint8_t ScaleValueDetectChange;
char StrScaleValueDetect[6];
char StrPWMValue[6];
char StrOCR[6];
char DebugAskAnswer[16];
uint8_t PWMChanged;
char *StrScaleDetectptr;
char *StrPWMValueptr;
char *StrOCRptr;



int main(void)
{
	SW_RX_Init(); 
	BL_Init(MYUBRR);
	PWM_Init();
		
    while (1) 
    {
		if(SWrxDataPending)
			{
				SW_RX_Fill_Buffer();
			}
		if(SWmesIsComplete)
			{

			SW_GetMessage();
			ScaleValue = atof(SWscaleValueForBL+1);
			
			//OCR2A = ScaleValue;
			if ((ScaleValue > 0) && (ScaleValue != ScaleValueChange))
			{
				ScaleValueChange = ScaleValue;
				BL_SendStr (SWscaleValueForBL);
				
				StrScaleDetectptr = shift_and_mul_utoa16 (ScaleValueDetect, StrScaleValueDetect) - 1;
				*StrScaleDetectptr = 's';
				BL_SendStr(StrScaleDetectptr);
				
				StrPWMValueptr = shift_and_mul_utoa16 (PWMvalue, StrPWMValue) - 1;
				*StrPWMValueptr = 'p';
				BL_SendStr(StrPWMValueptr);
				
				StrOCRptr = shift_and_mul_utoa16 (OCR2A, StrOCR) - 1;
				*StrOCRptr = 'o';
				BL_SendStr(StrOCRptr);
				
			}
			SWmesIsComplete = 0;		
			
			}
				
			
						
		if (BLmesIsComplete) 
			{
			//BL_SendStr (BluetoothMessage);
						
			BL_DefComd(); // defining gotten message from bluetooth (smartphone)
			BLmesIsComplete = 0;  // reset flag "complete message from smartphone"
			}
			
		if(DebugAsk)
			{
				StrScaleDetectptr = shift_and_mul_utoa16 (ScaleValueDetect, StrScaleValueDetect) - 1;
				*StrScaleDetectptr = 's';
				BL_SendStr(StrScaleDetectptr);
				
				StrPWMValueptr = shift_and_mul_utoa16 (PWMvalue, StrPWMValue) - 1;
				*StrPWMValueptr = 'p';
				BL_SendStr(StrPWMValueptr);
				
				StrOCRptr = shift_and_mul_utoa16 (OCR2A, StrOCR) - 1;
				*StrOCRptr = 'o';
				BL_SendStr(StrOCRptr);

				DebugAsk = 0;
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

