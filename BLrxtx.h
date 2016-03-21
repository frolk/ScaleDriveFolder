
#ifndef BLrxtx_H_
#define BLrxtx_H_

#include <avr/io.h>

#define FOSC 16000000 /* Clock Speed */
#define BAUD 57600
#define MYUBRR FOSC/16/BAUD-1
#define SIZE_BUF 32

extern uint8_t BLmesIsComplete;
extern uint8_t DebugAsk;

extern uint8_t BLrxBuf[SIZE_BUF];
extern uint8_t rxBufTail;
extern uint8_t rxBufHead;
extern uint8_t rxCount;

extern uint8_t BLtxBuf[SIZE_BUF];
extern uint8_t txBufTail;
extern uint8_t txBufHead;
extern uint8_t txCount;
extern char BlrxChar;
extern uint8_t BLlongMsg;



extern void BL_Init(uint16_t ubrr); // initiate fast PWM mode for changing OCnX voltage value

extern void BL_FlushRxBuf(void); // flush rx buffer: head, tail, count = 0

uint8_t BL_GetChar(void); // get one symbol from buffer using rxhead pointer

void BL_FlushTxBuf(void); // flush tx buffer: head, tail, count = 0

void BL_PutChar(char sym); // put one symbol into buffer using txhead pointer

extern void BL_SendStr(char *data); // send string starting from data address

extern void BL_PutOneByte(uint8_t value);

#endif

