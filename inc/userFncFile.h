#ifndef USERFNCFILE_H
#define USERFNCFILE_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

// includes type definitions
#include "streamOut.h"
#include "streamIn.h"

/* Function Prototypes */
void onReset();
void eI2C(char* tag, const streamIn_t* const msg);
void eUSB(char* tag, const streamIn_t* const msg);
void etOut1();
void etOut1_awaitPoll(void);

#endif /* USERFNCFILE_H */

