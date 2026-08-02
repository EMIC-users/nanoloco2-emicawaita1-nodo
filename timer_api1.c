#include <xc.h>
#include "inc/systemTimer.h"
#include "inc/userFncFile.h"


static uint32_t time_ms = 0; 
static uint32_t setPoint_ms = 0; 
static char modo;


void setTime1(uint32_t setPoint, char l_modo)
{
	setPoint_ms = setPoint;
	modo = l_modo;
	time_ms =  getSystemMilis();
}

void timer1_Poll(void)
{
	if ( setPoint_ms > 0 &&  getSystemMilis() - time_ms > setPoint_ms)
	{	
		if(modo == 'T')
		{
			setPoint_ms = 0;
		}
		else
		{
			time_ms = getSystemMilis();
		}
		etOut1();
	}
}


