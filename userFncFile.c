#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "inc/userFncFile.h"
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

void etOut1()
{
    LEDs_led_state(2);
}



