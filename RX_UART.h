#ifndef RX_UART_H_
#define RX_UART_H_

//Message from scale
#define SCALE_VALUE "v"
#define SCALE_DETECT_VALUE "d"


// baudrate
#define BR_9600
#define SW_SIZE_BUF 18

#ifdef  BR_9600
#define TICKSFIRST          51  //!< Ticks between two bits.
#define TICKS2WAITONE       103  //!< Wait one bit period.
#endif

#define INTERRUPT_EXEC_CYCL   9
// register, pins defines
#define ENABLE_TIMER_INTERRUPT( )       ( TIMSK0 |= ( 1<< OCIE0A ) )
#define DISABLE_TIMER_INTERRUPT( )      ( TIMSK0 &= ~( 1<< OCIE0A ) )
#define CLEAR_TIMER_INTERRUPT( )        ( TIFR0 |= ((1 << OCF0A) ) )
#define ENABLE_EXTERNAL0_INTERRUPT( )   ( EIMSK|= ( 1<< INT1 ) )
#define DISABLE_EXTERNAL0_INTERRUPT( )  ( EIMSK &= ~( 1<< INT1 ) )
#define SW_RX_PIN						PORTD3               //!< Receive data pin, must be INT0

#define GET_RX_PIN()					( PIND & (1 << SW_RX_PIN))

// Type defined enumeration hoding software UART's state
// main variables, data, counters, typedef of states
// External interrupt service routine

void SW_FlushRxBuf();
void SW_RX_Init(void);
void SW_RX_Fill_Buffer(void);
extern void SW_GetMessage(void);
extern void SW_GetScaleValue();
//extern void SW_Define_Data(void);


static volatile unsigned char SW_RX_Data;     //!< Storage for received bits.
static volatile unsigned char SW_RX_Bit_Count; //!< RX bit counter.

extern char SWrxMessage[18];
extern volatile uint8_t SWrxDataPending;
static volatile uint8_t SWrxBuf[SW_SIZE_BUF];
static volatile uint8_t SWrxBufTail;
static volatile uint8_t SWrxBufHead;
static volatile uint8_t SWrxCount;// Receive variables
extern volatile uint8_t SWmesIsComplete;
extern char SWscaleValueForBL[10];
extern volatile float ScaleValue;
//extern volatile float ScaleValue;


#endif /* RX_UART_H_ */