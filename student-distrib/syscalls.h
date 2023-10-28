#ifndef _SYS_CALLS_
#define _SYS_CALLS_

#include "types.h"

int32_t system_execute(const uint8_t* command);
int32_t system_halt(uint8_t status);

#endif
