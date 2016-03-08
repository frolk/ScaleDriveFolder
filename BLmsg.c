#include <stdlib.h>
#include <avr/io.h>
#include "BLmsg.h"
#include "BLrxtx.h"
#include "RX_UART.h"
#include <avr/interrupt.h>


#define	SEND_OCR1A		StrOCRptr1 = IntToStrKey(OCR1A, StrOCR1, 'o', ',');\
						BL_SendStr(StrOCRptr1);

char BluetoothMessage[10];
float DiscretValue;

//Parameters for sending via BL
uint16_t PWMvalue1= 0;
char *StrPWMvalueptr1;
char StrPWMvalue1[7];

uint16_t PWMvalue2= 0;
char *StrPWMvalueptr2;
char StrPWMvalue2[7];

uint32_t ScaleNPV;


float ScaleValueChange;
uint16_t ScaleValueDetect;
char StrScaleValueDetect[7]; 
char StrOCR1[7];
char StrOCR2[7];
char *StrScaleDetectptr;
char *StrOCRptr1;
char *StrOCRptr2;

uint8_t PWMValueChanged1 = 0;
uint8_t PWMValueChanged2 = 0; 

uint8_t DefineScaleMode = 0;
uint16_t TimerOVF_count; 
uint8_t TimerOVF_countFinish = 0;
uint8_t DefinedDiscret = 0;


ISR(TIMER1_OVF_vect)
{
	TimerOVF_count++;
	if (TimerOVF_count == 800)
	{
		TimerOVF_countFinish = 1;		
	}
	
}

void DefineScale()
{
// one time set up settings for defining scale mode
	if (DefineScaleMode == 1)
	{
	BL_SendStr("Starting...");
	UCSR0B &= ~ (1 << RXCIE0)|(1 << RXEN0); // disable rx interrupt while define scale
	TIMSK1 |= (1 << TOIE1);
	TCNT1 = 0; // reset timer1
	TimerOVF_count = 0;
	DefineScaleMode = 2;  // set up mode "in process"
	
	} 
	
// if spent 0.2ms time try to get OCR value of new ScaleValue	
	if (TimerOVF_countFinish == 1)
		{
			TimerOVF_countFinish = 0;
			TimerOVF_count = 0;
			if ((ScaleValue > 0) && (ScaleValue != ScaleValueChange) && (ScaleValue > ScaleValueChange))
				{
					ScaleValueChange = ScaleValue;
					StrOCRptr1 = IntToStrKey(OCR1A, StrOCR1, 'o', ',');
					BL_SendStr(StrOCRptr1);
					
					if(!DefinedDiscret)
						{
						DiscretValue = ScaleValue - ScaleValueChange;
						DefinedDiscret = 1;
						BL_SendStr ("d");
						} // if first time changed ScaleValue, i.e got discret
					
					BL_SendStr (SWscaleValueForBL);
					BL_SendStr("\n\r");
					
					
				}

			OCR1A++; // increase PWM signal on optocoupler
		}
	
	if (OCR1A >= 500)
	{
		BL_SendStr("defined");
		BluetoothMessage[0] = 'f'; //disable executing DefineScale function
		DefineScaleMode = 0;  // reset flag
		PWM_Init(); // return timer1 setting to default
		UCSR0B |= (1 << RXCIE0)|(1 << RXEN0);
		TIMSK1 &= ~(1 << TOIE1);
		//SEND_OCR1A;
		BL_SendStr("Bl[0]");
		BL_SendStr(BluetoothMessage);
	}
	
	
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

char* IntToStrKey(uint16_t val, char *buffer, char key1, char key2)
{
	char *str0;
	char *str1;
	char *str2;
	str2 = IntToStr (val, buffer);
	str0 = str2 - 2;
	str1 = str2 - 1;
	*str0 = key1;
	*str1 = key2;
	return str0;
}

void PWM_Init()
{
	DDRB = (1 << PORTB1)|(1 << PORTB3)|(1 << PORTB5);// OC1A, OC2A and ledPin like OUTPUT
	
	/* Timer2 for % correction */
	TCCR2A = (1 << WGM21)|(1 << WGM20)|(1<< COM2A1); // FastPWM mode for Timer2
	TCCR2B = (1<<CS20); // Start Timer2 with prescaler 1
	OCR2A = 0x00; // Reset Compare register OCR of Timer2
	/* Timer1 for kg correction */
	TCCR1A = (1 << COM1A1)|(1 << WGM11)|(1 << WGM10);
	TCCR1B = (1 << WGM12)|(1 << CS10);
	OCR1A = 0x0000;
}

void PWM_PinValue1()
{
	//OCR2A = PWMvalue;
	OCR1A = PWMvalue1;
	StrOCRptr1 = IntToStrKey(OCR1A, StrOCR1, 'o', ',');
	if(StrOCR1_lastValue != OCR1A)
	{
		BL_SendStr(StrOCRptr1);
		StrOCR1_lastValue = OCR1A;
	}
}

void PWM_PinValue2()
{
	//OCR2A = PWMvalue;
	OCR2A = PWMvalue2;
	StrOCRptr2 = IntToStrKey(OCR2A, StrOCR2, 'c',',');
	if(StrOCR2_lastValue != OCR2A)
	{
		BL_SendStr(StrOCRptr2);
		StrOCR2_lastValue = OCR2A;
		
	}
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
	if (BLmesIsComplete)
		{
			BL_GetMessage(); //pulling up buffer's data one by one
			if ((BluetoothMessage[0] == '-')|(BluetoothMessage[0] == '+'))
				{
					PWMvalue1= atoi(BluetoothMessage+1);
					StrPWMvalueptr1 = IntToStrKey(PWMvalue1, StrPWMvalue1, 'p', ',');
					BL_SendStr(StrPWMvalueptr1);
				}
			
			else if (BluetoothMessage[0] == 'c')
			{
				if (DefineScaleMode == 0)
				{
					DefineScaleMode = 1;  // first timer we entry into this function
				}
				DefineScale();				
			}
			
			
			BLmesIsComplete = 0;  // reset flag "complete message from smartphone"
		}
}

void BL_SendMsg()
	{
			if ((ScaleValue > 0) && (ScaleValue != ScaleValueChange) && (!DefineScaleMode))
			{
		
				ScaleValueChange = ScaleValue;
				StrScaleDetectptr = IntToStrKey(ScaleValueDetect, StrScaleValueDetect, 's', ',');
				BL_SendStr (SWscaleValueForBL);
				BL_SendStr(StrScaleDetectptr);
				BL_SendStr(StrOCRptr1);
				BL_SendStr(StrOCRptr2);
				BL_SendStr(StrPWMvalueptr1);
				BL_SendStr("\n\r");
			}	
	}

void BL_SetCorrect()

 {
	
	if (PWMvalue1 && (ScaleValue > 20))
		{
			PWM_PinValue1();   // write gotten correction value from smartphone to OCR2A for change OC2A pin PWM
			ScaleValueDetect = ScaleValue;
		}

	if((ScaleValue < (ScaleValueDetect - 2)) && (ScaleValue > 5) && (!DefineScaleMode))
		{
			OCR1A = 0;
			OCR2A = 0;
			ScaleValueDetect = 0;
			StrOCRptr1 = IntToStrKey(OCR1A, StrOCR1, 'o', ',');
			StrScaleDetectptr = IntToStrKey(ScaleValueDetect, StrScaleValueDetect, 's', ',');
		}

 }
 
 
 
 
 
 
 
 
 
 
 
 
 
 