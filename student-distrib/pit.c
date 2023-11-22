#include "pit.h"
#include "terminal.h"
#include "syscalls.h"

int timer = 0;

void init_pit(){
    /* Use channel 0 and a square wave generator. */
    int hz = RATE; /* We will divide the Input Clock's frequency with this value to get an IRQ every 5 milliseconds*/
    int divisor = INPUT_CLOCK_HZ / hz;       /* Calculate our divisor */
    outb(SET_CHANNEL_0, COMMAND_REGISTER);             /* Set our command byte 0x36 */
    outb(divisor & LOW_BYTE, CHANNEL_0);   /* Set low byte of divisor */
    outb(divisor >> HIGH_BYTE, CHANNEL_0);     /* Set high byte of divisor */

    /* Enables the IRQ of the PIT*/
    enable_irq(PIT_IRQ);
}

void pit_handler()
{   
    //pause everything and save previous terminal information
    curr_terminal = (curr_terminal+1)%3; 
    send_eoi(PIT_IRQ);
}
