#include "pit.h"
#include "terminal.h"
#include "syscalls.h"
#include "page.h"

/* init_pit
 * DESCRIPTION: Initializes the PIT by enabling IRQ0 on the PIC, turning on square wave interrupts on the pit,
 *                and setting the PIC frequency to the intended rate.
 * Inputs: none
 * Outputs: none
 * Return Value: none
 * Function: Allows for the PIT to send periodic interrupts.
 */
void init_pit() {
    /* Use channel 0 and a square wave generator. */
    int divisor = INPUT_CLOCK_HZ / RATE;       /* Calculate our divisor by dividing the max frequency of the PIT by the rate that we want. */
    outb(SET_CHANNEL_0, COMMAND_REGISTER);             /* Set our command byte 0x36 */
    outb(divisor & LOW_BYTE, CHANNEL_0);   /* Set low byte of divisor */
    outb(divisor >> HIGH_BYTE, CHANNEL_0);     /* Set high byte of divisor */
    base_shell = 1;

    /* Enables the IRQ of the PIT*/
    enable_irq(PIT_IRQ);
}

/* pit_handler
 * DESCRIPTION: Function called by IDT through PIT interrupts that calls scheduler helper function.
 * Inputs: none
 * Outputs: none
 * Return Value: none
 * Function: Allows for round robin scheduling
 */
void pit_handler() {   
    // Call the scheduler
    cli();
    send_eoi(PIT_IRQ);
    scheduler();
}

/* scheduler
 * DESCRIPTION: Function called by pit_handler that handls all our round-robin scheduling logic.
 * Inputs: none
 * Outputs: none
 * Return Value: none
 * Function: Switches between processes and executes base shells of terminal 1 and 2.
 */
void scheduler() {
    pcb_t* old_pcb = get_pcb(terminal_array[curr_terminal].pid);
    pcb_t* next_pcb;
    int next_pid = -1;
    
    // Temp variables to hold ebp and esp
    uint32_t temp_esp;
    uint32_t temp_ebp;

    //move to next scheduled terminal (0->1->2->0->....)
    curr_terminal = (curr_terminal + 1) % MAX_TERMINALS;
    
    /* Getting the ebp and esp of the current terminal. */
    asm volatile("                     \n\
          movl %%ebp, %0               \n\
          movl %%esp, %1               \n\
      "
          : "=r" (temp_ebp), "=r" (temp_esp)
          :
          : "eax"
          );

    /* Storing the ebp and esp of the current terminal onto the stack. */
    old_pcb->ebp = temp_ebp;
    old_pcb->esp = temp_esp;

    /* Opening a new shell if the flag is set to 0. */
    if (terminal_array[curr_terminal].flag == 0) {
        clear();
        terminal_array[curr_terminal].flag = 1;
        base_shell = 1;
        system_execute((uint8_t *) "shell");
        
    }

    // get youngest process id of new terminal
    next_pid = terminal_array[curr_terminal].pid;
    
    if(curr_terminal == screen_terminal){ //if current terminal being handled is screen terminal give vidmap addr to actual screen vid mem
        vid_map[0].base_addr = (int) (VIDEO_ADDR / ALIGN); 
    }
    else{ // else give vidmap addr to respective backup page
        vid_map[0].base_addr = (int) (VIDEO_ADDR / ALIGN) + (curr_terminal+1);
    }
    
    //get pcb of next process and handle process paging
    next_pcb = get_pcb(next_pid);
    process_page(next_pid);
    flushTLB();

    // Restoring tss
    tss.esp0 = terminal_array[curr_terminal].base_tss_esp0;
    tss.ss0 = terminal_array[curr_terminal].base_tss_ss0;

    //going to stack of new terminal
    asm volatile("                           \n\
                movl %0, %%esp               \n\
                movl %1, %%ebp               \n\
                "
                :
                : "r" (next_pcb->esp), "r" (next_pcb->ebp)
                : "eax"
                );
                
    sti();
}

