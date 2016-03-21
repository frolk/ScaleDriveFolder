
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#include "Blrxtx.h"
#include "BLmsg.h"


uint8_t BLtxBuf[SIZE_BUF];
uint8_t txBufTail = 0;
uint8_t txBufHead = 0;
uint8_t txCount = 0; // Transmit variables
uint8_t DebugAsk = 0;

uint8_t BLrxBuf[SIZE_BUF];
uint8_t rxBufTail = 0;
uint8_t rxBufHead = 0;
uint8_t rxCount = 0;// Receive variables
uint8_t BLmesIsComplete;
char BlrxChar = '\0';
uint8_t BLlongMsg = 0;



ISR (USART_RX_vect)   // receive interrupt routine
{
	BlrxChar = UDR0;
	
	if((BlrxChar == '/')||(BlrxChar == '!'))
	{
		BLlongMsg = 1;
	}
	
	if((BlrxChar == 0x26) || ((BlrxChar == 0x20) && (rxBufTail >= 3))) 
	{
		BLmesIsComplete = 1;
	}
	
	if ((rxCount < SIZE_BUF) && (BLlongMsg == 1))
	{
		BLrxBuf[rxBufTail] = BlrxChar;
		rxBufTail++;
		if(rxBufTail == SIZE_BUF) rxBufTail = 0;
		rxCount++;
		
	}
}

ISR (USART_TX_vect)   // transmit interrupt routine
{
	if(txCount > 0)
	{
		UDR0 = BLtxBuf[txBufHead]; // take one byte from buffer and put it into transmit register
		txCount--;
		txBufHead++;
		if(txBufHead == SIZE_BUF) txBufHead = 0; 
		
	}
}

void BL_Init(uint16_t ubrr) // initialize UART
{
	
	//set baud rate = 9600bps
	UBRR0H = (uint8_t)(ubrr>>8);
	UBRR0L = (uint8_t)ubrr;
	
	//set 1 stop bit, no parity bit and 8 bit character size
	UCSR0C = 0x06;  // (1<<UCSZ01)|(1<<UCSZ00)
	
	//enable transmission and reception 0x78
	UCSR0B |= (1<<RXCIE0)|(1<<TXCIE0)|(1<<RXEN0)|(1<<TXEN0);
	
	sei();
}

void BL_PutChar(char sym)// write next symbol into ring buffer
{
	if (((UCSR0A & (1<<UDRE0)) != 0) && (txCount == 0)) UDR0 = sym; // if udr is empty and it's first byte from
																	// ring buffer we write it directly into udr
    else
	{
		
		if (txCount < SIZE_BUF)
		{
			
			BLtxBuf[txBufTail] = sym; // second and the next bytes we write into ring buffer with txBufTaail pointer
			txCount++;  // increase variable that tells us how much unread data locate in ring buffer
			txBufTail++; // increase our pointer
			if (txBufTail == SIZE_BUF) txBufTail = 0; // if pointer reached end of buffer - reset it
	
		}
	}
}

void BL_PutOneByte(uint8_t value)
{
	if ((UCSR0A & (1<<UDRE0)) != 0) UDR0 = value;
}


void BL_SendStr(char *data)// send string start from the first member with address pointed by *data
{
	char sym;
	while(*data) // while data isn't '\0' or while data consist any data
	{
		sym = *data++; // write consisting value of data into sym local variable
		BL_PutChar(sym); // call function of putting every value into the ring buffer 
	}
	if (*data == '\0')
	{
		BL_PutChar(',');
	}
}

void BL_FlushRxBuf(void)  // flush our ring buffer after getting all of the array's member in out CurrentValue variable
{
	rxBufTail = 0;
	rxBufHead = 0;
	rxCount = 0;
	BLlongMsg = 0;
}

uint8_t BL_GetChar(void) // take one symbol from buffer using the Head pointer
{
	uint8_t sym;
	if(rxCount > 0)
	{
		sym	= BLrxBuf[rxBufHead];
		rxCount --;
		rxBufHead++;
		if(rxBufHead == SIZE_BUF) rxBufHead = 0;
		return sym;
	}
	return 0;
}

