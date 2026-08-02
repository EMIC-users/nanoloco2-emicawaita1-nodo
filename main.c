#include <xc.h>
#include "inc/systemConfig.h"

#include "inc/systemTimer.h"
#include "inc/led_led.h"
#include "inc/timer_api1.h"
#include "inc/timer_api2.h"
#include "inc/timer_api3.h"
#include "inc/timer_api4.h"
#include "inc/timer_api5.h"
#include "inc/timer_api6.h"
#include "inc/conversionFunctions.h"
#include "inc/USB_API.h"
#include "inc/frame_MSG.h"
#include "inc/emicbus2_core.h"
#include "inc/Persist.h"
#include "inc/await.h"
#include "inc/system.h"
#include "inc/userFncFile.h"

#include "system.c"

int main(void)
{
	initSystem();
	systemTimeInit();
	LEDs_led_init();
	USB_Init();
	EMICBus2_init();
	onReset();
	do
	{
		LEDs_led_poll();
		timer1_Poll();
		Poll_USB();
		poll_EMICBus2();
		await_poll();
	}
	while(1);
}

