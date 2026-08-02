#include "inc/gpio.h"
#include "inc/MCP2200.h"
#include "inc/UART1.h"

void Init_USBDriver()
{
	#ifdef PORT_MCP2200_RST
    HAL_GPIO_PinCfg(MCP2200_RST,GPIO_OUTPUT);
    HAL_GPIO_PinSet(MCP2200_RST,GPIO_HIGH);
	#endif

  UART1_init();
}


void Poll_USBDriver(void)
{
}

