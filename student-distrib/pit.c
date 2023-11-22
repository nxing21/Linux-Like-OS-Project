#include "pit.h"
#include "terminal.h"
#include "syscalls.h"

int timer = 0;

void init_pit(){
    /* Use channel 0 and a square wave generator. */
    int hz = RATE; /* We will divide the Input Clock's frequency with this value to get an IRQ every 10 milliseconds*/
    int divisor = INPUT_CLOCK_HZ / hz;       /* Calculate our divisor */
    outb(SET_CHANNEL_0, COMMAND_REGISTER);             /* Set our command byte 0x36 */
    outb(divisor & LOW_BYTE, CHANNEL_0);   /* Set low byte of divisor */
    outb(divisor >> HIGH_BYTE, CHANNEL_0);     /* Set high byte of divisor */

    // At the start, only the first terminal (terminal 0) will be active
    active_terminals[0] = 1;
    active_terminals[1] = 0;
    active_terminals[2] = 0;

    /* Enables the IRQ of the PIT*/
    enable_irq(PIT_IRQ);
}

void pit_handler()
{   
    //pause everything and save previous terminal information
    // curr_terminal = (curr_terminal+1) % MAX_TERMINALS;
    send_eoi(PIT_IRQ);
    // Call the scheduler
    scheduler();
}

void scheduler() {
    // do stuff
    curr_terminal = (curr_terminal + 1) % MAX_TERMINALS;
    while (active_terminals[curr_terminal] == 0) {
        curr_terminal = (curr_terminal + 1) % MAX_TERMINALS;
    }
    // First time opening this terminal, need to call execute shell
    if (active_terminals[screen_terminal] == 0) {
        active_terminals[screen_terminal] = 1;
        system_execute((uint8_t *) "shell");
    }
}
