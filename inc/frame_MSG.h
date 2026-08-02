#ifndef _EMICBUS2_FRAME_MSG_H_
#define _EMICBUS2_FRAME_MSG_H_
#include "streamIn.h"

void pI2C(char* format_, ...);
extern void eI2C(char* tag, const streamIn_t* const msg);

#endif

