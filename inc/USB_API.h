
#ifndef _USB_API_H_
#define _USB_API_H_

/*==================[public functions definition]==========================*/
void USB_Init(void);


void pUSB(char* format,...);

void Poll_USB(void);

void ISR_UART1_CALLBACK(char d);

#endif
