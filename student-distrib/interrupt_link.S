#define ASM     1


.text


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
