
/*==================[inclusions]=============================================*/
#include "inc/USB_API.h"
#include <xc.h>
#include "inc/MCP2200.h"
#include "inc/streamIn.h"
#include "inc/UART1.h"
#include <stdarg.h>
#include "inc/userFncFile.h"


#define USBFrameLf '\n'

static uint8_t frameFlagUSB = 0;

void USB_Init()
{
	Init_USBDriver();
}


void pUSB(char* format,...)
{
	va_list arg;
    va_start(arg, format);

	sendDataToStream(&streamOut_Uart1,format,arg);

	va_end(arg);
	UART1_OUT_push(USBFrameLf); 

}


void Poll_USB(void)
{
	if(frameFlagUSB)
	{
		frameFlagUSB = 0;
		char tag[20];
		char d;
		uint8_t i = 0;
		d = UART1_IN_pop();
		while (!UART1_is_empty(&UART1_IN_fifo) && d != '\t' && i < 20)
		{
			tag[i++] = d;
			d = UART1_IN_pop();
		}
		tag[i] = 0;
		eUSB(tag,&streamIn_Uart1);
		while(!UART1_is_empty(&UART1_IN_fifo))
		{
			d = UART1_IN_pop();
		}	
	}

}

void ISR_UART1_CALLBACK(char d)
{
	if(d == USBFrameLf)
	{
		frameFlagUSB = 1;
		return;
	}
	UART1_push(&UART1_IN_fifo, d) ;		//Push data into the input buffer
}


uint16_t USB_sendCount(void)
{
	return UART1_count(&UART1_OUT_fifo);
}

