
#ifndef _INTERRUPT_WRAPPER_H_
#define _INTERRUPT_WRAPPER_H_

#include "keyboard.h"
#include "rtc.h"
#include "pit.h"

#ifndef ASM

/* Linkage functions for each type of interrupt. */
void keyboard_handler_linkage();
void rtc_handler_linkage();
void pit_handler_linkage();

#endif

#endif
