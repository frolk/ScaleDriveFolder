
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "RX_UART.h"

static volatile unsigned char SW_RX_Data;     //!< Storage for received bits.
static volatile unsigned char SW_RX_Bit_Count; //!< RX bit counter.

static volatile uint8_t SWrxBuf[SW_SIZE_BUF];
static volatile uint8_t SWrxBufTail = 0;
static volatile uint8_t SWrxBufHead = 0;
static volatile uint8_t SWnumBit;    // What's number of bit. It's need for delaying
static volatile uint8_t SWrxCount = 0;// Receive variables
volatile uint8_t SWrxMessage[18];
volatile uint8_t SWrxDataPending;
volatile uint8_t SWmesIsComplete;
char SWscaleValueForBL[10];
volatile float ScaleValue;

void SW_FlushRxBuf()
{
	SWrxCount = 0;
	SWrxBufHead = 0;
	SWrxBufTail = 0;
	
}

ISR (INT0_vect)
{
	SWnumBit = 0;
	DISABLE_EXTERNAL0_INTERRUPT ();		// disable interrupt during the data bits
	
	DISABLE_TIMER_INTERRUPT();			//disable timer to change its registers
	TCCR0B &= ~( 1 << CS01 );			//reset prescaler counter
	
	TCNT0 = INTERRUPT_EXEC_CYCL;		// Clear counter register. Include time to run interrupt rutine
	
	TCCR0B |= ( 1 << CS01 );			// Start prescaler clock
	
	OCR0A = TICKSFIRST;			// Count one and half period into the future
	
	SW_RX_Bit_Count = 0;				// Clear received bit counter.
	
	
	ENABLE_TIMER_INTERRUPT();			// Enable timer0 interrupt on again
	CLEAR_TIMER_INTERRUPT();			// Clear interrupt bits
}

ISR (TIMER0_COMPA_vect)
{
	if(!SWnumBit)
	{
		SWnumBit = 1;
		OCR0A = TICKS2WAITONE;  // Count one period after the falling edge is trigged
	}
	else
	{
		OCR0A = TICKS2WAITONE;  // Count one period after the falling edge is trigged
		// Receiving lsb first
		if( SW_RX_Bit_Count < 8 )
		{
			SW_RX_Bit_Count++;
			SW_RX_Data = (SW_RX_Data>>1); // Shift due to receiving lsb first
			if ( GET_RX_PIN() != 0 )
			{
				SW_RX_Data |= 0x80;		  // If a logical 1 is read, let the data mirror this
			}
		}
		else
		{
			SWrxDataPending = 1;		// Enter data pending when one byte is received
			DISABLE_TIMER_INTERRUPT();	// Disable this interrupt
			EIFR |= (1 << INTF0);	// Reset flag not to enter the ISR one extra time
			ENABLE_EXTERNAL0_INTERRUPT(); // Enable interrupt to receive more bytes.
		}
	 }
}

void SW_RX_Init(void)
{
	//PORT
	PORTD |= ( 1<<SW_RX_PIN);	// RX_PIN is input, tri-stated
	
	// Timer0
	DISABLE_TIMER_INTERRUPT();
	TCCR0A = 0x00;		// Reset control register
	TCCR0B = 0x00;		// Reset presclaler
	TCCR0A |= (1 << WGM01);  //Timer in CTC mode
	TCCR0B |= (1 << CS01);	 //divide by 8 prescaler
	
	//External interrupt
	EIFR= 0x00;				//Init
	EICRA |= ( 1<<ISC01 );	//Interrupt sense control: falling edge
	ENABLE_EXTERNAL0_INTERRUPT();  // Turn external interrupt on
	
	CLEAR_TIMER_INTERRUPT();
	//Internal State Variable
	SWrxDataPending = 0;
	sei();
	
}

void SW_RX_Fill_Buffer(void)
{
		if(SWrxCount < SW_SIZE_BUF)
		{
			SWrxBuf[SWrxBufTail] = SW_RX_Data;
			SWrxDataPending = 0;
			if (SWrxBuf[SWrxBufTail] == 0x0A)
			{
				SWmesIsComplete = 1;
			}
			SWrxBufTail++;
			if(SWrxBufTail == SW_SIZE_BUF) SWrxBufTail = 0;
			SWrxCount++;
		}
}

char SW_GetChar(void)
{
	char SWsym;
	if (SWrxCount > 0)
	{
		SWsym = SWrxBuf[SWrxBufHead];
		SWrxCount--;
		SWrxBufHead++;
		if(SWrxBufHead == SW_SIZE_BUF) SWrxBufHead = 0;
		return SWsym;
	}
	else
	{
		return 0;
	}

}

void SW_GetMessage(void)
{
	uint8_t j;
	uint8_t i;
	for (i=0, j=1; i<18; i++)
	{
		char SWvalue = SW_GetChar();
		SWrxMessage[i] = SWvalue;
		SWscaleValueForBL[0] = 'v';
		if ( (i>6) && (i<14) && (SWrxMessage[i] != ' '))
			{
				SWscaleValueForBL[j] = SWvalue;
				j++;
			}
		if (i==14)
		{
			SWscaleValueForBL[j] = 0x0D;
		}
	}
}

void SW_GetScaleValue(void)
{
	if(SWrxDataPending)
	{
		SW_RX_Fill_Buffer();
	}
	if(SWmesIsComplete)
	{

		SW_GetMessage();
		ScaleValue = atof(SWscaleValueForBL+1);
		SWmesIsComplete = 0;
	}
	
}
	




