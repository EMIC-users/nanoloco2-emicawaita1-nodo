#include <xc.h>
#include "inc/await.h"
#include "inc/systemTimer.h"
#include "inc/userFncFile.h"

void await_poll(void)
{
	etOut1_awaitPoll();
}

