#include <stdlib.h>
#include <avr/io.h>
#include "BLmsg.h"
#include "BLrxtx.h"
#include "RX_UART.h"
#include <avr/eeprom.h>
#include <avr/interrupt.h>

char BluetoothMessage[16];
float DiscretValue;

//Parameters for sending via BL
uint16_t PWMvalue1 = 0;
char *StrPWMvalueptr1;
char StrPWMvalue1[7];

uint16_t PWMvalue2 = 0;
char *StrPWMvalueptr2;
char StrPWMvalue2[7];

uint32_t ScaleNPV;

float ScaleValueChange = 0;
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
uint8_t DefineKG = 0;
uint8_t DefinePercent = 0;
uint16_t TimerOVF_count;
uint8_t TimerOVF_countFinish = 0;
uint8_t DefinedDiscret = 0; // variables
uint8_t ValueCorrect = 0;
uint8_t ReverseDirection = 0;
uint16_t LastOCR1A = 0;
uint16_t TimerOVF_count_Max = 0;
uint8_t countOfResetZero = 0;
uint8_t ResetScale = 0;
uint16_t TimeOfReset = 0;
//uint16_t LengthOfHold = 0;
//uint16_t WhenHold = 0;
uint8_t SendAddTimes = 0;
uint16_t OCR1A_Max;
uint16_t OCR1B_Max;
uint16_t MinCorrectValue = 0;
uint16_t TurnCorrectOff = 0;
uint8_t UpChange = 0;
uint8_t ChangeDirection = 0;
uint8_t DetectOCR1B;
uint8_t SetValueMode = 0;
uint8_t ButValue = 0;


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

ISR(TIMER1_OVF_vect)
{
	TimerOVF_count++;
	if (TimerOVF_count == TimerOVF_count_Max)
	{
		TimerOVF_countFinish = 1;
	}

}

void Start_TimerOVFcount(uint16_t MaxValue)
	{
		TIMSK1 |= (1 << TOIE1);
		TCNT1 = 0; // reset timer1
		TimerOVF_count = 0;
		TimerOVF_count_Max = MaxValue;		
	}
	
	

void DefineScale()
{
	// one time set up settings for defining scale mode
	if (DefineScaleMode == 1)
	{
		//UCSR0B &= ~(1 << RXCIE0) | (1 << RXEN0); // disable rx interrupt while define scale
		Start_TimerOVFcount(900);
		DefineScaleMode = 2;  // set up mode "in process"

	}

	// if spent 0.2ms time try to get OCR value of new ScaleValue
	if ((TimerOVF_countFinish == 1) && (DefineScaleMode == 2))
	{
		TimerOVF_countFinish = 0;
		TimerOVF_count = 0;
		
		
		if ((ScaleValue > 0) && (ScaleValue > ScaleValueChange) && (DefineKG == 1))
		{
			StrOCRptr1 = IntToStrKey(OCR1A, StrOCR1, 'k', ',');
			BL_SendStr(SWscaleValueForBL);
			BL_SendStr(StrOCRptr1);
			BL_SendStr("\n\r");
		}
		
		if(DefineKG == 1)
		{
			OCR1A++; // increase PWM signal on optocoupler
		}
			
		if (DefinePercent == 1)
		{
		
				if ((ChangeDirection == 1) && (UpChange == 0))
				{
					if (ScaleValue < ScaleValueChange)
					{
						ChangeDirection = 0;
					} 
					else if (ScaleValue > ScaleValueChange)
					{
						UpChange = 1;
					}
				}
				
				//else if ((ChangeDirection == 1) && (UpChange == 1))
				//{
					//if (ScaleValue > ScaleValueChange)
					//{
						//ChangeDirection = 0;
					//}
					//else if (ScaleValue < ScaleValueChange)
					//{
						//UpChange = 0;
					//}
				//}
				
		
			if (((ScaleValue >= 0) && (ValueCorrect == 1)) && ((UpChange == 0) || ((UpChange == 1) && (ChangeDirection == 1))))
			
			{
				ValueCorrect = 0;
				ReverseDirection = 0;
				
				StrOCRptr2 = IntToStrKey(OCR1B, StrOCR2, '%', ',');
				StrOCRptr1 = IntToStr(OCR1A, StrOCR1);
				BL_SendStr(StrOCRptr2);
				BL_SendStr("o");
				BL_PutChar('+');
				BL_SendStr(StrOCRptr1);
				BL_SendStr("\n\r");
				OCR1A = 0;
				DetectOCR1B = 0;
			}
			
			if ((ScaleValue <= 0) && (ValueCorrect == 1) && (UpChange == 1) && (ChangeDirection == 0))
				
			{
				ValueCorrect = 0;
				ReverseDirection = 0;
				
				StrOCRptr2 = IntToStrKey(OCR1B, StrOCR2, '%', ',');
				StrOCRptr1 = IntToStr(OCR1A, StrOCR1);
				BL_SendStr(StrOCRptr2);
				BL_SendStr("o");
				BL_PutChar('-');
				BL_SendStr(StrOCRptr1);
				BL_SendStr("\n\r");
				OCR1A = 0;
				}
				
				
			if ((ScaleValue > ScaleValueChange) && (UpChange == 1) && (ReverseDirection == 0))
			{
				if ((ScaleValueChange < 0) && (DetectOCR1B == 1))
				{
					CORRECT_INIT_PLUS
					OCR1A = LastOCR1A;
					ReverseDirection = 1;
					ScaleValueChange = ScaleValue;
					
					
				}
				else if (ScaleValueChange > 0)
				{
					CORRECT_INIT_MINUS
					OCR1A = LastOCR1A*0.95;
					ReverseDirection = 1;
					ScaleValueChange = ScaleValue;
				}
				
				
			}
			
			
			if (ReverseDirection == 0)
				{
					OCR1B++;
					DetectOCR1B = 1;
					if (ScaleValue == ScaleValueChange)
					{
						ChangeDirection = 1;
					}
					
				}
		
			
				
			if ((ScaleValue < ScaleValueChange) && (UpChange == 0) && (ReverseDirection == 0))
			{
					CORRECT_INIT_PLUS
					OCR1A = LastOCR1A*0.95;
					ReverseDirection = 1;
					ScaleValueChange = ScaleValue;
			}
			
			
			
			
			
			if (ReverseDirection == 1)
				if (UpChange == 0)
				{
					OCR1A++;
					ValueCorrect = 1;
					LastOCR1A = OCR1A;
				}
				
				if ((UpChange == 1) && (ScaleValue < 0))
				{
					OCR1A--;
					ValueCorrect = 1;
					LastOCR1A = OCR1A;
				}
				if ((UpChange == 1) && (ScaleValue > 0))
				{
					OCR1A++;
					ValueCorrect = 1;
					LastOCR1A = OCR1A;
				}
				
				
		}
		
	}
	if ((OCR1A >= OCR1A_Max) && (DefineKG == 1))
	
	{
		DefineScaleMode = 0;
		DefineKG = 0;
		OCR1A = 0;
		TimerOVF_count = 0;
		BL_SendStr("k,f,k,f,k,f");
	
		BL_FlushRxBuf();
		//RESET;
	}
	
	if (((OCR1B > OCR1B_Max) && (DefinePercent == 1)) || (OCR1A > 1000))
	{
		BL_SendStr("%1,f,%1,f");
		DefineScaleMode = 0;
		DefinePercent = 0;
		OCR1B = 0;
		OCR1A = 0;
		TimerOVF_count = 0;
		BL_FlushRxBuf();
	}


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
	//if (*str0 == '0')
	//{
		//str0 = str0 + 1;
	//}
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
	TCCR1A = (1 << COM1A1) |(1 << COM1B1) |(1 << WGM10) |(1 << WGM11);
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
	OCR1B = PWMvalue2;
	StrOCRptr2 = IntToStrKey(OCR1B, StrOCR2, 'o', ',');
	if (StrOCR2_lastValue != OCR1B)
	{
		BL_SendStr(StrOCRptr2);
		StrOCR2_lastValue = OCR1B;
	}
}

void BL_GetMessage() // getting value from ring buffer to BlutoothMessage array
{

	for (int i = 0; i<16; i++)
	{
		BluetoothMessage[i] = BL_GetChar();
	}
	
}

void ResetCorrect()
{
	PWMvalue1 = 0;
	PWMvalue2 = 0;
	OCR1A = 0;
	OCR1B = 0;
}

void BL_DefComd()
{
	if (ButValue)
	{
		if (TimerOVF_countFinish == 1)
		{
			ButValue = 0;	
			TimerOVF_count = 0;
			TimerOVF_countFinish = 0;
			PORTD |= (1 << PORTD4) |(1 << PORTD5) |	(1 << PORTD6) |	(1 << PORTD7);
		}
	}
	if (ResetScale == 1)
	{
		if (TimerOVF_countFinish == 1)
		{
			if (countOfResetZero < TimeOfReset)
				{	
					if (countOfResetZero & 1)
						{
							PORTD |= (1 << PORTD7);
						} 
					else
						{
							PORTD &= ~(1 << PORTD7);
						}
						//
					//if (countOfResetZero == LengthOfHold)
						//{
							//PORTD |= (1 << PORTD6);
						//}
					//if (countOfResetZero == WhenHold)
						//{
						   //PORTD &= ~(1 << PORTD6);
						//}
					//if (countOfResetZero == (WhenHold+LengthOfHold))
						//{
							//PORTD &= ~(1 << PORTD6);
						//}
						//
					//if (countOfResetZero == (WhenHold+2*LengthOfHold))
					//{
							//PORTD |= (1 << PORTD6);
//
					//}//hold things //hold
					countOfResetZero++;	
					TimerOVF_count = 0;
					TimerOVF_countFinish = 0;
				}
			else
				{
					TIMSK1 &=  ~(1 << TOIE1);
					countOfResetZero = 0;
					
					ResetScale = 0;
					PORTD |= (1 << PORTD7); // zero pin 
					//PORTD |= (1 << PORTD6); //hold pin

				}
			
		}
	}
	
	
	if (BLmesIsComplete == 1)
	{
		BL_GetMessage();
			
		if ((BluetoothMessage[0] == '$') && (BluetoothMessage[1] == '+'))
		{
			CORRECT_INIT_PLUS
			PWMvalue1 = atoi(BluetoothMessage + 2);
			StrPWMvalueptr1 = IntToStrKey(PWMvalue1, StrPWMvalue1, 'p', ',');
			BL_SendStr("+");
			BL_SendStr(StrPWMvalueptr1);
		}
		else if ((BluetoothMessage[0] == '$') && (BluetoothMessage[1] == '-'))
		{
			CORRECT_INIT_MINUS
			PWMvalue1 = atoi(BluetoothMessage + 2);
			StrPWMvalueptr1 = IntToStrKey(PWMvalue1, StrPWMvalue1, 'p', ',');
			BL_SendStr("-");
			BL_SendStr(StrPWMvalueptr1);
		}
			
		else if ((BluetoothMessage[0] == '$') && (BluetoothMessage[1] == '%'))
		{
			
			if (BluetoothMessage[2] == 'z')
			{
				
				PWMvalue2 = atoi(BluetoothMessage + 3);
				
				Start_TimerOVFcount(TimerOVF_count_Max);
				ResetScale = 1;
				//BL_SendStr("z%");
			}
			
			if (BluetoothMessage[2] == 'a')
			{
				CORRECT_INIT_PLUS
				PWMvalue2 = atoi(BluetoothMessage + 3);
				//BL_SendStr("a%");
			}
			
			
			StrPWMvalueptr2 = IntToStrKey(PWMvalue2, StrPWMvalue2, 'p', ',');
			
			BL_SendStr(StrPWMvalueptr2);
			
			
		}
					
		else if ((BluetoothMessage[0] == '^') && (BluetoothMessage[1] == 'c') && (DefineScaleMode == 0))
		{
				CORRECT_INIT_PLUS
				DefineKG = 1;
				DefineScaleMode = 1;  // first time we entry into this function
				OCR1A_Max = atoi (BluetoothMessage + 2);
				DefineScale();
		}
		
		else if ((BluetoothMessage[0] == '^') && (BluetoothMessage[1] == 'e') && (DefineScaleMode == 0))
		{
			ResetCorrect();	
		}
		
		
		else if ((BluetoothMessage[0] == '^') && (BluetoothMessage[1] == 's') && (DefineScaleMode == 0))
		{
			SetValueMode = atoi(BluetoothMessage + 2);
			
		}
		
		else if ((BluetoothMessage[0] == '^') && (BluetoothMessage[1] == 'm') && (DefineScaleMode == 0)) // set up min correct value
		{
			MinCorrectValue = atoi (BluetoothMessage + 2);
				BL_SendStr("CorrOn - ok");
			
		}
		
		else if ((BluetoothMessage[0] == '^') && (BluetoothMessage[1] == 'd') && (DefineScaleMode == 0)) // set up min correct value
		{
			TurnCorrectOff = atoi (BluetoothMessage + 2);
			BL_SendStr("CorrOff - ok");
		}
						
		else if ((BluetoothMessage[0] == '^') && (BluetoothMessage[1] == 'z') && (DefineScaleMode == 0))
		{
			TimerOVF_count_Max = 600; //atoi (BluetoothMessage + 2)
			TimeOfReset = 20;
			ResetScale = 1;
			Start_TimerOVFcount(TimerOVF_count_Max);

			BL_SendStr("z%");
			//LengthOfHold = atoi (BluetoothMessage + 9);
			//WhenHold = atoi (BluetoothMessage + 11);
			//PORTD &= ~ (1 << PORTD6);
			
			
		}
		
		else if ((BluetoothMessage[0] == '^') && (BluetoothMessage[1] == '4') && (DefineScaleMode == 0))
		{
			TimerOVF_count_Max = 8000; //atoi (BluetoothMessage + 2)
			Start_TimerOVFcount(TimerOVF_count_Max);
			PORTD &= ~(1 << PORTD4);
			BL_SendStr("ON/OFF");
			TimerOVF_count = 0;
			TimerOVF_countFinish = 0;
			ButValue = 1;
			
			
		}
		else if ((BluetoothMessage[0] == '^') && (BluetoothMessage[1] == '5') && (DefineScaleMode == 0))
		{
			TimerOVF_count_Max = 700; //atoi (BluetoothMessage + 2)
			Start_TimerOVFcount(TimerOVF_count_Max);
			PORTD &= ~(1 << PORTD5);
			BL_SendStr("HOLD");
			TimerOVF_count = 0;
			TimerOVF_countFinish = 0;
			ButValue = 1;
		}
		else if ((BluetoothMessage[0] == '^') && (BluetoothMessage[1] == '6') && (DefineScaleMode == 0))
		{
			TimerOVF_count_Max = 700; //atoi (BluetoothMessage + 2)
			Start_TimerOVFcount(TimerOVF_count_Max);
			PORTD &= ~(1 << PORTD6);
			BL_SendStr("TARE");
			TimerOVF_count = 0;
			TimerOVF_countFinish = 0;
			ButValue = 1;
		}
		else if ((BluetoothMessage[0] == '^') && (BluetoothMessage[1] == '7') && (DefineScaleMode == 0))
		{
			TimerOVF_count_Max = 700; //atoi (BluetoothMessage + 2)
			Start_TimerOVFcount(TimerOVF_count_Max);
			PORTD &= ~(1 << PORTD7);
			BL_SendStr("TARE");
			TimerOVF_count = 0;
			TimerOVF_countFinish = 0;
			ButValue = 1;
		}
		
		
		else if ((BluetoothMessage[0] == '^') && (BluetoothMessage[1] == '%') && (DefineScaleMode == 0))
		{
				//CORRECT_INIT_PLUS
				DefinePercent = 1;
				DefineScaleMode = 1;  // first time we entry into this function
				OCR1B_Max = atoi (BluetoothMessage + 2);
				ScaleValueChange = ScaleValue;
				DefineScale();
				
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
	
	if ((!DefineScaleMode) && (ScaleValue != ScaleValueChange) && (ScaleValue != 0) && (DebugAsk == 0))	
	{
		
		BL_SendStr(SWscaleValueForBL);
		
		
		ScaleValueChange = ScaleValue;
	}
		
	else if ((!DefineScaleMode) && (ScaleValue != ScaleValueChange) && (ScaleValue > 0) && (DebugAsk == 1))	
		{
		ScaleValueChange = ScaleValue;
		BL_SendStr(SWscaleValueForBL);
		StrScaleDetectptr = IntToStrKey(ScaleValueDetect, StrScaleValueDetect, 's', ',');
		}
}

void BL_SetCorrect()

{	
	if (((PWMvalue1)||(PWMvalue2)) && (!DefineScaleMode) && (ScaleValue < TurnCorrectOff) && (ScaleValue > 0)) // && (!SetValueMode))
	{
		OCR1A = 0;
		OCR1B = 0;
	}
	
	//if (SetValueMode == 1) //(ScaleValue >= MinCorrectValue) &&
	//{
		//if (PWMvalue1)
		//{
			//PWM_PinValue1();
 	//
		//}
		//if (PWMvalue2)
		//{
			//PWM_PinValue2();
		//}
	//}
	
	if ((PWMvalue1) && (!DefineScaleMode) && (ScaleValue >= MinCorrectValue)) //(ScaleValue >= MinCorrectValue) && 
	{
		PWM_PinValue1();   // write gotten correction value from smartphone to OCR2A for change OC1A pin PWM
		ScaleValueDetect = ScaleValue;
	}
	if  ((PWMvalue2) && (!DefineScaleMode) && (ScaleValue >= MinCorrectValue)) //&& (ScaleValue >= MinCorrectValue)
	{
		PWM_PinValue2();   // write gotten correction value from smartphone to OCR2A for change OC1B pin PWM
		ScaleValueDetect = ScaleValue;
	}
	
	
//
	//if ((ScaleValue < (ScaleValueDetect - 2)) && (ScaleValue > 5) && (!DefineScaleMode))
	//{
		//OCR1A = 0;
		//OCR1B = 0;
		//ScaleValueDetect = 0;
		//StrOCRptr1 = IntToStrKey(OCR1A, StrOCR1, 'o', ',');
		//StrScaleDetectptr = IntToStrKey(ScaleValueDetect, StrScaleValueDetect, 's', ',');
	//}

}



