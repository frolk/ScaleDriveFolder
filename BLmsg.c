
#include <stdlib.h>
#include <avr/io.h>
#include "BLmsg.h"
#include "BLrxtx.h"
#include <avr/interrupt.h>



//uint32_t dutyCycle = 0;

uint8_t BluetoothMessage[10];
uint16_t PWMvalue = 0;

//void ConvParamStrWithKey(int param, char* addBuffer, char key)
//{
	//
	//StrScaleDetectptr = shift_and_mul_utoa16 (, StrScaleValueDetect) - 1;
	//*StrScaleDetectptr = 's';
//}


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
	
	BL_GetMessage(); //pulling up buffer's data one by one
	if ((BluetoothMessage[0] == '-')|(BluetoothMessage[0] == '+'))
	{
		PWMvalue = atoi(BluetoothMessage+1); //convert our string into float integer
	} 
}

