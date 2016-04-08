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
uint16_t PWMvalue1 = 0;
char *StrPWMvalueptr1;
char StrPWMvalue1[7];

uint16_t PWMvalue2 = 0;
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
uint8_t DefinedDiscret = 0; // variables


ISR(TIMER1_OVF_vect)
{
	TimerOVF_count++;
	if (TimerOVF_count ==900)
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
		UCSR0B &= ~(1 << RXCIE0) | (1 << RXEN0); // disable rx interrupt while define scale
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
		if ((ScaleValue > 0) && (ScaleValue > ScaleValueChange))
		{
			//if ((DiscretValue == 0) || (ScaleValueChange < ScaleValue))
			//{
				//DiscretValue = ScaleValue - ScaleValueChange;
				//DefinedDiscret = 1;
			//}
			//ScaleValueChange = ScaleValue;
			BL_SendStr("d,");
			StrOCRptr1 = IntToStrKey(OCR1A, StrOCR1, 'o', ',');
			BL_SendStr(StrOCRptr1);
			BL_SendStr(SWscaleValueForBL);
			BL_SendStr("\n\r");
			
			//if (((ScaleValue - ScaleValueChange) < DiscretValue) || (DefinedDiscret == 1))
			//{
				//DiscretValue = ScaleValue - ScaleValueChange;
				//DefinedDiscret = 1;
				//
			//} // if first time changed ScaleValue, i.e got discret

		}

		OCR1A++; // increase PWM signal on optocoupler
	}
	if (OCR1A >= 800)
	{
		BL_SendStr("df");
		BL_SendStr("df");
		BL_SendStr("df");
		BL_SendStr("df");
		BL_SendStr("df");
		RESET;
	}


}

char* IntToStr(uint16_t n, char *buffer)
{
	uint8_t d4, d3, d2, d1, q, d0;

	d1 = (n >> 4) & 0xF;
	d2 = (n >> 8) & 0xF;
	d3 = (n >> 12) & 0xF;

	d0 = 6 * (d3 + d2 + d1) + (n & 0xF);
	q = (d0 * 0xCD) >> 11;
	d0 = d0 - 10 * q;

	d1 = q + 9 * d3 + 5 * d2 + d1;
	q = (d1 * 0xCD) >> 11;
	d1 = d1 - 10 * q;

	d2 = q + 2 * d2;
	q = (d2 * 0x1A) >> 8;
	d2 = d2 - 10 * q;

	d3 = q + 4 * d3;
	d4 = (d3 * 0x1A) >> 8;
	d3 = d3 - 10 * d4;

	char *ptr = buffer;
	*ptr++ = (d4 + '0');
	*ptr++ = (d3 + '0');
	*ptr++ = (d2 + '0');
	*ptr++ = (d1 + '0');
	*ptr++ = (d0 + '0');
	*ptr = 0;

	while (buffer[0] == '0') ++buffer;
	return buffer;
}

char* IntToStrKey(uint16_t val, char *buffer, char key1, char key2)
{
	char *str0;
	char *str1;
	char *str2;
	str2 = IntToStr(val, buffer);
	str0 = str2 - 2;
	str1 = str2 - 1;
	*str0 = key1;
	*str1 = key2;
	return str0;
}

void PWM_Init()
{
	DDRB |= (1 << PORTB1) |(1 << PORTB2) | (1 << PORTB3) | (1 << PORTB4) | (1 << PORTB5);// OC1A, OC2A and ledPin like OUTPUT
	//PORTB |= (1 << PORTB4);
	/* Timer2 for % correction */
	TCCR2A = (1 << WGM21) | (1 << WGM20) | (1 << COM2A1); // FastPWM mode for Timer2
	TCCR2B = (1 << CS20); // Start Timer2 with prescaler 1
	OCR2A = 0x00; // Reset Compare register OCR of Timer2
	/* Timer1 for kg correction */
	TCCR1A = (1 << COM1A1) |(1 << COM1B1) |  (1 << WGM11) | (1 << WGM10);
	TCCR1B = (1 << WGM12) | (1 << CS10);
	OCR1A = 0x0000;
	OCR1B = 0x0000;
}

void PWM_PinValue1()
{
	//OCR2A = PWMvalue;
	OCR1A = PWMvalue1;
	StrOCRptr1 = IntToStrKey(OCR1A, StrOCR1, 'o', ',');
	if (StrOCR1_lastValue != OCR1A)
	{
		BL_SendStr(StrOCRptr1);
		StrOCR1_lastValue = OCR1A;
	}
}

void PWM_PinValue2()
{
	//OCR2A = PWMvalue;
	OCR1B = PWMvalue2;
	StrOCRptr2 = IntToStrKey(OCR2B, StrOCR2, 'o', ',');
	if (StrOCR2_lastValue != OCR2B)
	{
		BL_SendStr(StrOCRptr2);
		StrOCR2_lastValue = OCR2B;
	}
}

void BL_GetMessage() // getting value from ring buffer to BlutoothMessage array
{

	for (int i = 0; i<8; i++)
	{
		BluetoothMessage[i] = BL_GetChar();
	}
	
}

void BL_DefComd()
{
	if (BLmesIsComplete == 1)
	{
		BL_GetMessage();
		if ((BluetoothMessage[0] == '$') && (BluetoothMessage[1] == '+'))
		{
			PORTB |= (1 << PORTB5);
			PORTC |= (1 << PORTC0);
			PORTC |= (1 << PORTC1);
			PORTC &= ~(1 << PORTC2);
			PWMvalue1 = atoi(BluetoothMessage + 2);
			StrPWMvalueptr1 = IntToStrKey(PWMvalue1, StrPWMvalue1, 'p', ',');
			BL_SendStr("+");
			BL_SendStr(StrPWMvalueptr1);
		}
		else if ((BluetoothMessage[0] == '$') && (BluetoothMessage[1] == '-'))
		{
			PORTB |= (1 << PORTB5);
			PORTC &= ~(1 << PORTC0);
			PORTC |= (1 << PORTC1);
			PORTC &= ~(1 << PORTC2);
			PWMvalue1 = atoi(BluetoothMessage + 2);
			StrPWMvalueptr1 = IntToStrKey(PWMvalue1, StrPWMvalue1, 'p', ',');
			BL_SendStr("-");
			BL_SendStr(StrPWMvalueptr1);
		}
			
		else if ((BluetoothMessage[0] == '$') && (BluetoothMessage[1] == '%'))
		{
			PWMvalue2 = atoi(BluetoothMessage + 2);
			StrPWMvalueptr2 = IntToStrKey(PWMvalue2, StrPWMvalue2, 'p', ',');
			BL_SendStr("%");
			BL_SendStr(StrPWMvalueptr2);
			
			
		}
		
		else if ((BluetoothMessage[0] == '!') && (BluetoothMessage[1] == 0x30) && (BluetoothMessage[2] == 0x20) && (BluetoothMessage[3] == 0x30) && (BluetoothMessage[4] == 0x20)) // Reset MCU before starting firmware
		{
			RESET;
		}
		
		
			
		else if ((BluetoothMessage[0] == '^') && (BluetoothMessage[1] == 'c'))
		{
			BL_SendStr("ok");
			
			if (DefineScaleMode == 0)
			{
				PORTB |= (1 << PORTB5);
				PORTC |= (1 << PORTC0);
				PORTC |= (1 << PORTC1);
				PORTC &= ~(1 << PORTC2);
				DefineScaleMode = 1;  // first time we entry into this function
				DefineScale();
			}
		}
		
		
		else if ((BluetoothMessage[0] == '^') && (BluetoothMessage[1] == '%'))
		{
			BL_SendStr("ok");
			
			if (DefineScaleMode == 0)
			{
				PORTB |= (1 << PORTB5);
				PORTC |= (1 << PORTC0);
				PORTC |= (1 << PORTC1);
				PORTC &= ~(1 << PORTC2);
				DefineScaleMode = 1;  // first time we entry into this function
				DefineScale();
			}
		}
		
			
		else if (BluetoothMessage[0] == 'r')
		{
		
				switch (BluetoothMessage[1])
				{
					case 1: BL_PutOneByte(DDRB); break;
					case 2: BL_PutOneByte(DDRC); break;
					case 3: BL_PutOneByte(DDRD); break;
					case 4: BL_PutOneByte(EIFR); break;
					case 5: BL_PutOneByte(EIMSK); break;
					case 6: BL_PutOneByte(OCR0A); break;
					case 7: BL_PutOneByte(OCR0B); break;
					case 8: BL_PutOneByte(OCR1AH); break;
					case 9: BL_PutOneByte(OCR1AL); break;			
					case 10: BL_PutOneByte(OCR2A); break;
					case 11: BL_PutOneByte(OCR2B); break;
					case 12: BL_PutOneByte(OSCCAL); break;
					case 13: BL_PutOneByte(PORTB); break;
					case 14: BL_PutOneByte(PORTC); break;
					case 15: BL_PutOneByte(PORTD); break;
					case 16: BL_PutOneByte(TCCR0A); break;
					case 17: BL_PutOneByte(TCCR0B); break;
					case 18: BL_PutOneByte(TCCR1A); break;
					case 19: BL_PutOneByte(TCCR1B); break;
					case 20: BL_PutOneByte(TCCR1C); break;
					case 21: BL_PutOneByte(TCCR2A); break;
					case 22: BL_PutOneByte(TCCR2B); break;
					case 23: BL_PutOneByte(TCNT0); break;
					case 24: BL_PutOneByte(TCNT1H); break;
					case 25: BL_PutOneByte(TCNT1L); break;
					case 26: BL_PutOneByte(TCNT2); break;
					case 27: BL_PutOneByte(TIFR0); break;
					case 28: BL_PutOneByte(TIFR1); break;
					case 29: BL_PutOneByte(TIFR2); break;
					case 30: BL_PutOneByte(TIMSK0); break;
					case 31: BL_PutOneByte(TIMSK1); break;
					case 32: BL_PutOneByte(TIMSK2); break;
					case 33: BL_PutOneByte(UBRR0H); break;
					case 34: BL_PutOneByte(UBRR0L); break;
					case 35: BL_PutOneByte(UCSR0A); break;
					case 36: BL_PutOneByte(UCSR0B); break;
					case 37: BL_PutOneByte(UCSR0C); break;
					case 38: BL_PutOneByte(UDR0); break;
					case 39: BL_PutOneByte(WDTCSR); break;  // Send Registers
					case 40: BL_PutOneByte(WDTCSR); break;
					default: BL_SendStr("There are not");

				}
				
		
		}
		
	BLmesIsComplete = 0;  // reset flag "complete message from smartphone"
	BL_FlushRxBuf();  // flush our buffer and start from the beginning
}
}

void BL_SendMsg()
{
	if ((!DefineScaleMode) && (ScaleValue != ScaleValueChange) && (ScaleValue > 0) && (DebugAsk = 0))
	{
		ScaleValueChange = ScaleValue;
		BL_SendStr(SWscaleValueForBL);
	}
		
	else if ((!DefineScaleMode) && (ScaleValue != ScaleValueChange) && (ScaleValue > 0) && (DebugAsk = 1))	
		{
		ScaleValueChange = ScaleValue;
		BL_SendStr(SWscaleValueForBL);
		StrScaleDetectptr = IntToStrKey(ScaleValueDetect, StrScaleValueDetect, 's', ',');
		
		//BL_SendStr(StrScaleDetectptr);
		//BL_SendStr(StrOCRptr1);
		//BL_SendStr(StrOCRptr2);
		//BL_SendStr(StrPWMvalueptr1);
		//BL_SendStr("\n\r");
		}
}

void BL_SetCorrect()

{
	if (PWMvalue1 && (ScaleValue > 10))
	{
		PWM_PinValue1();   // write gotten correction value from smartphone to OCR2A for change OC2A pin PWM
		ScaleValueDetect = ScaleValue;
	}
	if (PWMvalue2 && (ScaleValue > 10))
	{
		PWM_PinValue2();   // write gotten correction value from smartphone to OCR2A for change OC2A pin PWM
		ScaleValueDetect = ScaleValue;
	}

	if ((ScaleValue < (ScaleValueDetect - 2)) && (ScaleValue > 5) && (!DefineScaleMode))
	{
		OCR1A = 0;
		OCR1B = 0;
		ScaleValueDetect = 0;
		StrOCRptr1 = IntToStrKey(OCR1A, StrOCR1, 'o', ',');
		StrScaleDetectptr = IntToStrKey(ScaleValueDetect, StrScaleValueDetect, 's', ',');
	}

}



