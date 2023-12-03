#define ASM     1

.text
/* INTR_LINK(name, func)
 * Inputs: name, func
 * Return Value: none
 * Function: This is assembly linkage to help the interrupts work properly.
 * Important because need to reach previous state.
 */
#define INTR_LINK(name, func)    \
    .globl name                 ;\
    name:                       ;\
        pushal                  ;\
        pushfl                  ;\
        call func               ;\
        popfl                   ;\
        popal                   ;\
        iret

INTR_LINK(keyboard_handler_linkage, keyboard_handler)
INTR_LINK(rtc_handler_linkage, RTC_handler)
INTR_LINK(pit_handler_linkage, pit_handler)



