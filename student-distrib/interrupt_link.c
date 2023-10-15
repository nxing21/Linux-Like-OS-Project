
#include "interrupt_link.h"
#include "idt.h"

#define INTR_LINK(name, func)    \
    .globl name                 ;\
    name:
        pushal                  ;\
        pushfl                  ;\
        call func               ;\
        popfl                   ;\
        popal                   ;\
        iret

INTR_LINK(keyboard_handler_linkage, keyboard_handler);
INTR_LINK(rtc_handler_linkage, rtc_handler);
