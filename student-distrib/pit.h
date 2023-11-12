#include "lib.h"
#include "i8259.h"

#define PIT_IRQ 0
#define CHANNEL_0 0x40

/* The PIT is used for scheduling. The reason that we don't use RTC is because the RTC is not deterministic.
User can change the RTC values whenever they want to, but they cannot change the PIT. */

/* Initalizes the Programmable Interrupt Timer. */
void init_pit();

void pit_handler();
