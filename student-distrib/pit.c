#include "pit.h"
#include "terminal.h"
#include "syscalls.h"
#include "page.h"
#include "init_devices.h"

int timer = 0;
extern int cur_processes[NUM_PROCESSES];

void init_pit() {
    /* Use channel 0 and a square wave generator. */
    int hz = RATE; /* We will divide the Input Clock's frequency with this value to get an IRQ every 10 milliseconds*/
    int divisor = INPUT_CLOCK_HZ / hz;       /* Calculate our divisor */
    outb(SET_CHANNEL_0, COMMAND_REGISTER);             /* Set our command byte 0x36 */
    outb(divisor & LOW_BYTE, CHANNEL_0);   /* Set low byte of divisor */
    outb(divisor >> HIGH_BYTE, CHANNEL_0);     /* Set high byte of divisor */
    base_shell = 1;

    /* Enables the IRQ of the PIT*/
    enable_irq(PIT_IRQ);
}

void pit_handler() {   
    //pause everything and save previous terminal information
    // curr_terminal = (curr_terminal+1) % MAX_TERMINALS;
    // send_eoi(PIT_IRQ);
    // Call the scheduler
    send_eoi(PIT_IRQ);
    scheduler();
}

void scheduler() {
    pcb_t* old_pcb = get_pcb(curr_pid);
    pcb_t* next_pcb;
    int next_pid = -1;
    // Temp variables to hold ebp and esp
    uint32_t temp_esp;
    uint32_t temp_ebp;

    // If it's the first time opening that terminal, we need to start the shell
    if (terminal_array[curr_terminal].flag == 0) {
        terminal_array[curr_terminal].flag = 1;
        send_eoi(KEYBOARD_IRQ);
        base_shell = 1;
        system_execute((uint8_t *) "shell");
    }
    
    // move to next terminal
    curr_terminal = (curr_terminal + 1) % MAX_TERMINALS;

    next_pid = terminal_array[curr_terminal].pid;

    //what if  terminal 0 and/or terminal 1 is using up all the processes TODO!!!

    // Grabbing ebp and esp to store for later context switching
    asm volatile("                           \n\
                movl %%ebp, %0               \n\
                movl %%esp, %1               \n\
                "
                : "=r" (temp_ebp), "=r" (temp_esp)
                :
                : "eax"
                );

    // store PCB's ebp and esp
    old_pcb->sched_ebp = temp_ebp;
    old_pcb->sched_esp = temp_esp;

    next_pcb = get_pcb(next_pid);
    process_page(next_pcb->pid);
    flushTLB();

    // Restoring tss
    tss.esp0 = next_pcb->tss_esp0;
    tss.ss0 = next_pcb->tss_ss0;

    //going to stack of new terminal
    asm volatile("                           \n\
                movl %0, %%esp               \n\
                movl %1, %%ebp               \n\
                "
                :
                : "r" (next_pcb->sched_esp), "r" (next_pcb->sched_ebp)
                : "eax"
                );
}
