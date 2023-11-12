#include "pit.h"

void init_pit(){
    /* Enables the IRQ of the PIT*/
    enable_irq(PIT_IRQ);
}

void pit_handler(){
    send_eoi(PIT_IRQ);
}
