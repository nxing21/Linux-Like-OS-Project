#include "lib.h"
#include "i8259.h"

#define PIT_IRQ 0
#define CHANNEL_0 0x40
#define INPUT_CLOCK_HZ 1193180
#define COMMAND_REGISTER 0x43
#define LOW_BYTE 0xFF
#define HIGH_BYTE 8
#define SET_CHANNEL_0 0x36
#define RATE 5965 /* Allows for the PIT to raise the IRQ about every 5 milliseconds */

/* The PIT is used for scheduling. The reason that we don't use RTC is because the RTC is not deterministic.
User can change the RTC values whenever they want to, but they cannot change the PIT. */

/* Initalizes the Programmable Interrupt Timer. */
void init_pit();

void pit_handler();
