
#ifndef BLdefComd_H_
#define BLdefComd_H_


#define RESET PORTC &= ~(1 << PORTC3)


extern uint16_t PWMvalue1;
extern uint16_t PWMvalue2;

extern void BL_DefComd();  // define bluetooth command

extern void PWM_Init();

extern void DefineScale();

extern void BL_SetCorrect();

void BL_GetMessage();

extern char BluetoothMessage[10];

void UART_Init (unsigned int ubrr); // initialize UART

char* IntToStrKey(uint16_t val, char *buffer, char key1, char key2);

extern char *StrPWMvalueptr1;
extern char *StrPWMvalueptr2;

char StrPWMvalue1[7];
char StrPWMvalue2[7];

char StrOCR1[7];
char *StrOCRptr1;
uint16_t StrOCR1_lastValue;

char StrOCR2[7];
char *StrOCRptr2;
uint8_t StrOCR2_lastValue;


float ScaleValueChange;
uint16_t ScaleValueDetect;
char StrScaleValueDetect[7];
char *StrScaleDetectptr;



extern uint8_t DefineScaleMode;
extern uint16_t TimerOVF_count;
extern uint8_t TimerOVF_countFinish;
extern uint8_t DefinedDiscret;

float DiscretValue;

extern void BL_SendMsg();


#endif 

