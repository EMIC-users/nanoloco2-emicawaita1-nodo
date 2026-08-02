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
#include "inc/await.h"

void onReset()
{
    setTime1(1000, 'A');
}


/* ==== EMIC-Await: evento etOut1 — FSM (reentrada LATEST) ==== */
typedef struct {
    uint8_t state;      /* 0 = idle; N = segmento que continua al despertar */
    uint32_t t_wake;    /* deadline de la espera activa (ms de getSystemMilis) */
    uint8_t pending;    /* invocacion llegada con la FSM ocupada (profundidad 1) */
    uint16_t overrun;   /* invocaciones sobrescritas (observabilidad) */
} etOut1_awaitCtx_t;
static etOut1_awaitCtx_t etOut1_awaitCtx;

static void etOut1_awaitFsm(void);

void etOut1()
{
    if (etOut1_awaitCtx.state != 0)
    {
        etOut1_awaitCtx.pending = 1;   /* LATEST: se relanza al terminar, con los ultimos args */
        etOut1_awaitCtx.overrun++;
        return;
    }
    etOut1_awaitCtx.state = 1;
    etOut1_awaitFsm();
}

void etOut1_awaitPoll(void)
{
    if (etOut1_awaitCtx.state == 0)
        return;
    if ((int32_t)(getSystemMilis() - etOut1_awaitCtx.t_wake) < 0)
        return;
    etOut1_awaitFsm();
}

static void etOut1_awaitFsm(void)
{
    switch (etOut1_awaitCtx.state)
    {
    case 1:
        LEDs_led_state(1);
        /* --- delay(500 ms): suspende la tarea hasta vencer la deadline --- */
        etOut1_awaitCtx.t_wake = getSystemMilis() + (uint32_t)(500);
        etOut1_awaitCtx.state = 2;
        return;
    case 2:
        LEDs_led_state(0);
    }
    /* fin del bloque: la tarea vuelve a idle */
    etOut1_awaitCtx.state = 0;
    if (etOut1_awaitCtx.pending != 0)
    {
        etOut1_awaitCtx.pending = 0;
        etOut1_awaitCtx.state = 1;
        etOut1_awaitCtx.t_wake = getSystemMilis();   /* relanza en la proxima vuelta del loop */
    }
}



