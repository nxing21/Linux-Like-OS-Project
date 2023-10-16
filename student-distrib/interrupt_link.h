
#ifndef _INTERRUPT_WRAPPER_H_
#define _INTERRUPT_WRAPPER_H_

#include "init_devices.h"
#include "rtc.h"

#ifndef ASM

void keyboard_handler_linkage();
void rtc_handler_linkage();

#endif

#endif
