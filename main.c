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


char* shift_and_mul_utoa16(uint16_t n, char *buffer)
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
				
				BL_SendStr(StrPWMValueptr);
				
				StrOCRptr = shift_and_mul_utoa16 (OCR2A, StrOCR) - 1;
				*StrOCRptr = 'o';
				BL_SendStr(StrOCRptr);
				
			}
			SWmesIsComplete = 0;		
			
			}
				
			
						
		if (BLmesIsComplete) 
			{
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
					StrPWMValueptr = shift_and_mul_utoa16 (PWMvalue, StrPWMValue) - 1;
				*StrPWMValueptr = 'p';
				}	
				
		if((ScaleValue < (ScaleValueDetect - 2)) && (ScaleValue > 5))
		{
			OCR2A = 0;
			ScaleValueDetect = 0;
		}
	}
}

