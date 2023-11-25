#include "pit.h"
#include "terminal.h"
#include "syscalls.h"
#include "page.h"

int timer = 0;
extern int cur_processes[NUM_PROCESSES];

void init_pit(){
    /* Use channel 0 and a square wave generator. */
    int hz = RATE; /* We will divide the Input Clock's frequency with this value to get an IRQ every 10 milliseconds*/
    int divisor = INPUT_CLOCK_HZ / hz;       /* Calculate our divisor */
    outb(SET_CHANNEL_0, COMMAND_REGISTER);             /* Set our command byte 0x36 */
    outb(divisor & LOW_BYTE, CHANNEL_0);   /* Set low byte of divisor */
    outb(divisor >> HIGH_BYTE, CHANNEL_0);     /* Set high byte of divisor */

    // // At the start, only the first terminal (terminal 0) will be active
    // terminal_array[0].flag = 1;
    // terminal_array[1].flag = 0;
    // terminal_array[2].flag = 0;
    // moved this part to init terminal

    /* Enables the IRQ of the PIT*/
    enable_irq(PIT_IRQ);
}

void pit_handler()
{   
    //pause everything and save previous terminal information
    // curr_terminal = (curr_terminal+1) % MAX_TERMINALS;
    send_eoi(PIT_IRQ);
    // Call the scheduler
    // scheduler();
}

void scheduler() {
    pcb_t* old_pcb = get_pcb(curr_pid);
    pcb_t* next_pcb;
    uint32_t next_pid = -1;
    int i;

    // Temp variables to hold ebp and esp
    uint32_t temp_esp;
    uint32_t temp_ebp;

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
    old_pcb->ebp = temp_ebp;
    old_pcb->esp = temp_esp;

    //move to next terminal *of the open terminals*
    // curr_terminal = (curr_terminal + 1) % MAX_TERMINALS;
    // while (terminal_array[curr_terminal].flag == 0) {
    //     curr_terminal = (curr_terminal + 1) % MAX_TERMINALS;
    // }


    // // First time opening this terminal, need to call execute shell
    // if (terminal_array[screen_terminal].flag == 0) {
    //     terminal_array[screen_terminal].flag = 1;
    //     system_execute((uint8_t *) "shell");
    // }


    //get pid of youngest child of terminal
    // for(i = 0; i < NUM_PROCESSES; i++) {
    //     next_pcb = get_pcb(i);
    //     if(cur_processes[i] != 0  && next_pcb->terminal_id == curr_terminal){
    //         next_pid = i;
    //     }
    // }

    next_pid = terminal_array[curr_terminal].pid;

    //what if  terminal 0 and/or terminal 1 is using up all the processes TODO!!!
    /*idk if this is correct*/
    // First time opening this terminal, need to call execute shell
    // if(next_pid == -1){
    //     terminal_array[curr_terminal].flag = 1;
    //     system_execute((uint8_t *) "shell");
    //     next_pid = curr_pid;
    //     next_pcb = get_pcb(next_pid);
    // }
    // else{
    //     next_pcb = get_pcb(next_pid);
    //     // Restore paging and flush TLB
    //     process_page(next_pcb->pid);
    //     flushTLB();
    // }

    next_pcb = get_pcb(next_pid);
    // Restore paging and flush TLB
    process_page(next_pid);
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
                : "r" (next_pcb->esp), "r" (next_pcb->ebp)
                : "eax"
                );
}
