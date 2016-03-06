
#ifndef BLdefComd_H_
#define BLdefComd_H_


extern uint16_t PWMvalue;

extern void BL_DefComd();  // define bluetooth command

extern void PWM_Init();

void BL_GetMessage();

extern uint8_t BluetoothMessage[10];

void UART_Init (unsigned int ubrr); // initialize UART

char* IntToStrKey(uint16_t val, char *buffer, char key);

extern char *StrPWMValueptr;

char StrPWMValue[6];


#endif 

