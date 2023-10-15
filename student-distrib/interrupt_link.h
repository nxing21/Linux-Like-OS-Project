
#ifndef _INTERRUPT_WRAPPER_H_
#define _INTERRUPT_WRAPPER_H_

#include "init_devices.h"

#ifndef ASM
    extern void keyboard_handler_linkage();
    extern void rtc_handler_linkage();
#endif

#endif
