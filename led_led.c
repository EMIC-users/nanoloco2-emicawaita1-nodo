#include <xc.h>
#include "inc/led_led.h"
#include "inc/gpio.h"
#include "inc/systemTimer.h"

void LEDs_led_init (void)
{
	HAL_GPIO_PinCfg(Led1, GPIO_OUTPUT);
}

void LEDs_led_state(uint8_t status)
{
	switch (status)
	{
		case 0:
			HAL_GPIO_PinSet(Led1, GPIO_LOW); // es mal
			break;
		case 1:
			HAL_GPIO_PinSet(Led1, GPIO_HIGH);
			break;
		case 2:
			if (HAL_GPIO_PinGet(Led1))
            {
				HAL_GPIO_PinSet(Led1,GPIO_LOW);
			}
			else 
			{
				HAL_GPIO_PinSet(Led1,GPIO_HIGH);
			}
			break;
	}
}

static uint16_t blkLed_timerOn = 0; 
static uint16_t blkLed_period = 0; 
static uint16_t blkLed_times = 0; 
static uint32_t blkLed_tStamp;
 
void LEDs_led_blink(uint16_t timeOn, uint16_t period, uint16_t times)
{
	HAL_GPIO_PinSet(Led1, GPIO_HIGH);
	blkLed_timerOn = timeOn; 
	blkLed_period = period; 
	blkLed_times = times;
	blkLed_tStamp = getSystemMilis();
}

void LEDs_led_poll ()
{
	if (blkLed_period > 0)
	{
		if ( getSystemMilis() - blkLed_tStamp > blkLed_period )
		{
			if (blkLed_times > 0)
			{
				blkLed_times--;
				if (blkLed_times == 0)
				{
					blkLed_period = 0;
				}
			}
			if (blkLed_period > 0)
			{
				HAL_GPIO_PinSet(Led1, GPIO_HIGH);
				blkLed_tStamp = getSystemMilis();
			}
		}
		else if ( getSystemMilis() - blkLed_tStamp > blkLed_timerOn )
		{
			HAL_GPIO_PinSet(Led1, GPIO_LOW);
		}
	}
}

