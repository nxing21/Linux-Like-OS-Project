#include "pit.h"
#include "terminal.h"
#include "syscalls.h"
#include "page.h"

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
    cli();
    pcb_t* old_pcb = get_pcb(terminal_array[curr_terminal].pid);
    pcb_t* next_pcb;
    int next_pid = -1;
    // Temp variables to hold ebp and esp
    uint32_t temp_esp;
    uint32_t temp_ebp;
    int temp_terminal;
    
    // move to next terminal
    temp_terminal = curr_terminal;
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
        terminal_array[curr_terminal].flag = 1;
        base_shell = 1;
        system_execute((uint8_t *) "shell");
        
    }


    next_pid = terminal_array[curr_terminal].pid;

    
    if(curr_terminal == screen_terminal){
        vid_map[0].base_addr = (int) (VIDEO_ADDR / ALIGN); // give vidmap pointer back
    }
    else{
        vid_map[0].base_addr = (int) (VIDEO_ADDR / ALIGN) + (curr_terminal+1);
    }
    

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

/* Adds a new program to the scheduler. */
void add_to_scheduler(int new_pid, int terminal_id){
    terminal_array[terminal_id].base_tss_esp0 = EIGHT_MB - new_pid * EIGHT_KB;
    terminal_array[terminal_id].base_tss_ss0 = KERNEL_DS;
}

void remove_from_scheduler(int new_pid, int terminal_id){
    terminal_array[terminal_id].base_tss_esp0 = EIGHT_MB - new_pid * EIGHT_KB;
    terminal_array[terminal_id].base_tss_ss0 = KERNEL_DS;
    terminal_array[terminal_id].pid = new_pid;
}

// the first terminal flag is set to 1 by default
// the first pid is not
