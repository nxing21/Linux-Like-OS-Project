#include "file_sys.h"
#include "rtc.h"
#include "syscalls.h"
#include "lib.h"
#include "page.h"
#include "x86_desc.h"
#include "terminal.h"
#include "pit.h"

int cur_processes[NUM_PROCESSES] = {0,0,0,0,0,0}; // cur_processes keeps track of current processes that are running


/* init_fops_table()
 * Inputs: none
 * Return Value: none
 * Function: sets correct terminal read/write function pointers
 */
void init_fops_table() {
    // Initializing stdout functions
    term_write_ops.open = NULL;
    term_write_ops.close = NULL;
    term_write_ops.write = &terminal_write;
    term_write_ops.read = NULL;

    // Initializing stdin functions
    term_read_ops.open = NULL;
    term_read_ops.close = NULL;
    term_read_ops.write = NULL;
    term_read_ops.read = &terminal_read;  

    // Initializing rtc functions
    rtc_ops.open = &RTC_open;
    rtc_ops.close = &RTC_close;
    rtc_ops.write = &RTC_write;
    rtc_ops.read = &RTC_read;

    // Initializing directory functions
    dir_ops.open = &open_directory;
    dir_ops.close = &close_directory;
    dir_ops.read = &read_directory;
    dir_ops.write = &write_directory;

    // Initializing file functions
    file_ops.open = &open_file;
    file_ops.close = &close_file;
    file_ops.read = &read_file;
    file_ops.write = &write_file;
}

/* system_execute(const uint8_t* command)
 * Inputs: const uint8_t* command: command inputted that we want to exceute if valid
 * Return Value: 0 (should never reach here), -1 (failure)
 * Function: Parses args, makes sure file is valid, sets up paging
 * for the process, and loads file into memory, creates PCB struct instance,
 * opens and initiliazes file descriptor array, 
 * prepares stack and context switch values, IRET
 */
int32_t system_execute(const uint8_t* command) {
    cli(); // clear interrupts
    int8_t elf_check[ELF_LENGTH]; // holds ELF that we want to compare with
    uint8_t filename[FILENAME_LEN + 1]; // holds name of executable we want to execute
    int8_t buf[ELF_LENGTH]; // holds info
    uint32_t pid; // process ID
    int arg_idx = 0; // start of arguments
    uint32_t temp_esp;
    uint32_t temp_ebp;
    char* cur_args = ""; // keeps track of current arguments inputted

    if (command == NULL) {
        sti();
        return -1;
    }
    int i; // looping variable
    int file_index; // looping variable for file index
    
    // Puts correct ELF info inside elf_check
    elf_check[DEL_INDEX] = DEL;
    elf_check[E_INDEX] = E;
    elf_check[L_INDEX] = L;
    elf_check[F_INDEX] = F;

    i = 0;
    file_index = 0;

    while (command[i] == ' ') { //skips initial spaces
        i++;
    }
    // Get the name of the executable
    while (command[i] != '\0' && i < FILENAME_LEN) {
        if (command[i] == ' ') {
            arg_idx = i + 1; //where the first arg potentially is
            while (command[arg_idx] == ' ') { //skipping spaces between executable name and first arg
                arg_idx++;
            }
            break;
        }
        else {
            filename[file_index] = command[i];
        }
        i++;
        file_index++;
    }
    filename[file_index] = '\0';

    i = 0;
    // two variables we will be using to check for spaces at the end of the argument
    int temp_idx;
    int space_flag;
    // Get arguments and putting it into global buffer
    while(arg_idx != 0 && arg_idx < strlen((int8_t*) command) && command[arg_idx] != '\0') {
        if (command[arg_idx] == ' ') {
            space_flag = 0; // space_flag being set to 0 means we are currently iterating through useless spaces at the end
            temp_idx = arg_idx; // temporary index since arg_idx needs to be saved for later
            while (command[temp_idx] != '\0') {
                if (command[temp_idx] != ' ') {
                    space_flag = 1; // if we found a non-space, it means the space we found originally is not at the end, so we set space_flag to 1
                    break;
                }
                temp_idx++; // keep iterating until end of string or we found a non-space
            }
            if (!space_flag) {
                // if we were at the end of the string, we don't want to add the useless spaces
                break;
            }
        }
        
        cur_args[i] = command[arg_idx]; // copy argument into cur_args
        // iterate index trackers
        arg_idx++;
        i++;
    }
    cur_args[i] = '\0';

    dentry_t dentry;
    // Check the validity of the filename
    if (read_dentry_by_name(filename, &dentry) == -1) {
        sti();
        return -1;
    }

    // Check ELF magic constant
    read_data(dentry.inode_num, 0, (uint8_t *) buf, ELF_LENGTH);
    if (strncmp(elf_check, buf, ELF_LENGTH) != 0) {
        sti();
        return -1;
    }

    /*--------------------------------------------------------------------------------------------------*/

    // Find free PID location
    for (i = 0; i <= NUM_PROCESSES; i++) {
        if (i == NUM_PROCESSES) {
            sti();
            return -1; // no available space for new process
        }
        else if (cur_processes[i] == 0) { // not in use process
            cur_processes[i] = 1;  // set to in use
            pid = i; // set pid
            break;
        }
    }

    // Set up paging and flush TLB
    process_page(pid);
    flushTLB();

    // User-level program loader
    read_data(dentry.inode_num, 0, (uint8_t *) VIRTUAL_ADDR, FOUR_MB);

    // Create PCB
    pcb_t *pcb = get_pcb(pid);
    pcb_t *parent_pcb;
    // Initialize PCB's pid
    pcb->pid = pid;
    // store pcb's arguments
    pcb->args = cur_args;

    // Check if base shell of the terminal it's on
    if (base_shell == 1) {
        pcb->parent_pid = BASE_SHELL;
        terminal_array[curr_terminal].pid = pid;
        pcb->terminal_id = curr_terminal;
    }
    else {
        // Set parent pcb to the pid in the terminal_array
        pcb->parent_pid = terminal_array[screen_terminal].pid;
        parent_pcb = get_pcb(terminal_array[screen_terminal].pid);

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
        parent_pcb->ebp = temp_ebp;
        parent_pcb->esp = temp_esp;
        terminal_array[screen_terminal].pid = pid;
        pcb->terminal_id = screen_terminal;
    }

    base_shell = 0;    

    // Initializing stdin
    pcb->file_descriptors[0].file_op_table_ptr = &term_read_ops;
    pcb->file_descriptors[0].inode = 0;
    pcb->file_descriptors[0].file_pos = 0;
    pcb->file_descriptors[0].flags = IN_USE;

    // Initializing stdout
    pcb->file_descriptors[1].file_op_table_ptr = &term_write_ops;
    pcb->file_descriptors[1].inode = 0;
    pcb->file_descriptors[1].file_pos = 0;
    pcb->file_descriptors[1].flags = IN_USE;

    // Initializing general use fd slots 
    for(i = FILE_DESCRIPTOR_MIN; i < FILE_DESCRIPTOR_MAX; i++) {
        pcb->file_descriptors[i].inode = 0;
        pcb->file_descriptors[i].file_pos = 0;
        pcb->file_descriptors[i].flags = NOT_IN_USE;
    }

    // Initialize PCB's tss variables
    pcb->tss_esp0 = tss.esp0;
    pcb->tss_ss0 = tss.ss0;

    // Context switch
    tss.esp0 = EIGHT_MB - pid * EIGHT_KB;
    tss.ss0 = KERNEL_DS;

    // Reads EIP (bytes 24-27)
    uint32_t eip;
    read_data(dentry.inode_num, 24, (uint8_t*)&eip, 4); // Magic numbers: 24 is the starting index, 4 is the length of bytes 24-27

    // Initialize PCB's eip
    pcb->eip = eip;

    // https://wiki.osdev.org/Getting_to_Ring_3
    
    update_tss(pid, curr_terminal); 

    // IRET
    // First line "0x02B is USER_DS"
    // Setting up stack for IRET context switch
    asm volatile("                                          \n\
                movw $0x2B, %%ax # user ds                  \n\
                movw %%ax, %%ds                             \n\
                pushl %0                                    \n\
                pushl %1                                    \n\
                pushfl                                      \n\
                popl %%eax                                  \n\
                orl $0x200, %%eax   # enabling interrupts   \n\
                pushl %%eax                                 \n\
                pushl %2                                    \n\
                pushl %3                                    \n\
                "
                :
                : "r" (USER_DS), "r" (USER_ESP), "r" (USER_CS), "r" (eip)
                : "eax"
                );
    asm volatile("iret");
    asm volatile("                  \n\
                execute_return:     \n\
                leave               \n\
                ret                 \n\
                ");

    return 0;
}

/* system_halt(uint8_t status)
 * Inputs: uint8_t status: return value set by user program
 * Return Value: never actually returns a value
 * Function: Restores parent pcb data,
 * restores parent pcb paging, checks if currently running shell,
 * if so calls execute("shell") again, else closes all relevant FDs,
 * and jumpts to execute return.
 */
int32_t system_halt(uint8_t status) {
    cli();
    int i; // looping variable

    // Get current and parent PCB
    int halting_pid = terminal_array[curr_terminal].pid;

    pcb_t* pcb = get_pcb(halting_pid);
    uint32_t parent_pid = pcb->parent_pid;
    pcb_t* parent_pcb = get_pcb(parent_pid);
    uint32_t ext_status;

    // If currently running base shell, reload
    if (parent_pid == BASE_SHELL && terminal_array[curr_terminal].flag == 1) {
        asm volatile("                                          \n\
                    cli                                         \n\
                    movw $0x2B, %%ax  # user ds                 \n\
                    movw %%ax, %%ds                             \n\
                    pushl %0                                    \n\
                    pushl %1                                    \n\
                    pushfl                                      \n\
                    popl %%eax                                  \n\
                    orl $0x200, %%eax   # enabling interrupts   \n\
                    pushl %%eax                                 \n\
                    pushl %2                                    \n\
                    pushl %3                                    \n\
                    "
                    :
                    : "r" (USER_DS), "r" (USER_ESP), "r" (USER_CS), "r" (pcb->eip)
                    : "eax"
                    );

        asm volatile("iret");
    }

    // Update cur_processes
    cur_processes[halting_pid] = 0;

    // Set the curr_pid to the parent pid.
    terminal_array[curr_terminal].pid = parent_pid;
    
    // Restore paging and flush TLB
    process_page(parent_pid);
    flushTLB();

    // Close all file operations
    for (i = 0; i < FILE_DESCRIPTOR_MAX; i++) {
        system_close(i);
    }

    // Restoring tss
    tss.esp0 = EIGHT_MB - parent_pid * EIGHT_KB;
    tss.ss0 = KERNEL_DS;

    if(status == EXCEPTION) { // accounting for status being 8 bits
        ext_status = EXCEPTION+1;
    }
    else{
        ext_status = status;
    }

    update_tss(parent_pid, curr_terminal);
    terminal_array[curr_terminal].pid = parent_pid;

    sti();
    // Assembly to load old esp, ebp, and status 
    asm volatile("                           \n\
                movl %0, %%eax               \n\
                movl %1, %%esp               \n\
                movl %2, %%ebp               \n\
                jmp execute_return           \n\
                "
                :
                : "r" (ext_status), "r" (parent_pcb->esp), "r" (parent_pcb->ebp)
                : "eax"
                );
    // Will never reach here
    return 0;
}

/* system_read (int32_t fd, void* buf, int32_t nbytes)
 * Inputs: int32_t fd: file descriptor index,
 * void* buf: buffer to be filled in with data,
 * int32_t nbytes: number of bytes to read
 * Return Value: Read function result, -1 ("failure")
 * Function: Makes sure fd index and the desciptor it points to is valid,
 * if so we call the corresponding read.
 */
int32_t system_read (int32_t fd, void* buf, int32_t nbytes) {
    pcb_t *pcb = get_pcb(terminal_array[curr_terminal].pid); // getting current pcb pointer
    if((fd >= 0 && fd < FILE_DESCRIPTOR_MAX && fd != 1) && pcb->file_descriptors[fd].flags != NOT_IN_USE) { 
        return pcb->file_descriptors[fd].file_op_table_ptr->read(fd, buf, nbytes); // returning respective read
    }
    else{
        return -1;
    }
}

/* system_write (int32_t fd, const void* buf, int32_t nbytes)
 * Inputs: int32_t fd: file descriptor index,
 * void* buf: buffer contains data to be written,
 * int32_t nbytes: number of bytes to written.
 * Return Value: Write function result, -1 ("failure")
 * Function: Makes sure fd index and the desciptor it points to is valid,
 * if so we call the corresponding write.
 */
int32_t system_write (int32_t fd, const void* buf, int32_t nbytes) {
    pcb_t *pcb = get_pcb(terminal_array[curr_terminal].pid); // getting current pcb pointer
    if((fd >= 1 && fd < FILE_DESCRIPTOR_MAX) && pcb->file_descriptors[fd].flags != NOT_IN_USE) { 
        return pcb->file_descriptors[fd].file_op_table_ptr->write(fd, buf, nbytes); // returning respective write
    }
    else{
        return -1;
    }
}

/* system_open (const uint8_t* filename)
 * Inputs: const uint8_t* filename: filename of file to be opened
 * Return Value: fd index opened, -1 ("failure")
 * Function: Makes sure fd index and the desciptor it points to is valid,
 * if so we call the corresponding open.
 */
int32_t system_open (const uint8_t* filename) {
    dentry_t temp_dentry;
    uint32_t file_type;
    int i;
    int index = -1;
    pcb_t *pcb = get_pcb(terminal_array[curr_terminal].pid); // getting current pcb pointer
    for(i = FILE_DESCRIPTOR_MIN; i < FILE_DESCRIPTOR_MAX; i++) { // finding first open file descriptor
        if(pcb->file_descriptors[i].flags == NOT_IN_USE) {
            index = i; // setting index of  open fd
            break;
        }
    }

    if((read_dentry_by_name(filename, &temp_dentry) != -1) && index != -1) { // check valid name and fds not full
        file_type = temp_dentry.filetype; // 0 for user-level access to RTC, 1 for the directory, and 2 for a regular file.
        pcb->file_descriptors[index].flags = IN_USE; // marking as in use
        pcb->file_descriptors[index].inode = temp_dentry.inode_num; // setting to correct inode
        pcb->file_descriptors[index].file_pos = 0; // initializing position to 0

        switch (file_type)
        {
            case 0: /* setting RTC functions */ 
                pcb->file_descriptors[index].file_op_table_ptr = &rtc_ops;
                break;

            case 1: /* setting directory functions */
                pcb->file_descriptors[index].file_op_table_ptr = &dir_ops;
                break;

            case 2: /*setting file functions*/
                pcb->file_descriptors[index].file_op_table_ptr = &file_ops;
                break;
            
            default:
                break;
        }
        pcb->file_descriptors[index].file_op_table_ptr->open(temp_dentry.filename);
        return index; // returning fd index of opened file descriptor
    }
    else {
        return -1;
    }
}

/* system_close (int32_t fd)
 * Inputs: int32_t fd: file descriptor index.
 * Return Value: Close function result, -1 ("failure")
 * Function: Makes sure fd index and the desciptor it points to is valid,
 * if so we call the corresponding close.
 */
int32_t system_close (int32_t fd) {
    pcb_t *pcb = get_pcb(terminal_array[curr_terminal].pid);
    // Check if fd is valid index and if fd is in use
    if ((fd >= FILE_DESCRIPTOR_MIN && fd < FILE_DESCRIPTOR_MAX) && pcb->file_descriptors[fd].flags != NOT_IN_USE) { 
        pcb->file_descriptors[fd].flags = NOT_IN_USE; // marking as not in use
        pcb->file_descriptors[fd].inode = -1; // marking as not pointing to any inode
        pcb->file_descriptors[fd].file_pos = 0; // file position reset to 0 
        return pcb->file_descriptors[fd].file_op_table_ptr->close(fd);
    } else {
        return -1;
    }
}

/* system_getargs(uint8_t* buf, int32_t nbytes)
 * Inputs: uint8_t* buf: buffer holding command line arguments, 
 * int32_t nbytes: bytes to be read.
 * Return Value: 0 ("success"), -1 ("failure")
 * Function: Reads the program’s command line arguments into a user-level buffer.
 */
int32_t system_getargs(uint8_t* buf, int32_t nbytes) {
    // cli();
    pcb_t *pcb = get_pcb(terminal_array[curr_terminal].pid);
    if(strlen(pcb->args) + 1 > nbytes || strlen(pcb->args) == 0) { //+1 to account for '\0' b/c strlen doesn't count it
        return -1;
    }
    else { //if checks pass copy current aargs into user buffer
        memcpy(buf, pcb->args, nbytes); 
        return 0;
    } 
    // sti();
}

/* system_vidmap(uint8_t** screen_start)
 * Inputs: uint8_t** screen_start: maps the text-mode video memory into user space at a pre-set virtual address
 * Return Value: 0 (success), -1 (failure)
 * Function: Sets up Video Map paging and gives user space access
 */
int32_t system_vidmap(uint8_t** screen_start) {
    
    if(screen_start == (uint8_t**) NULL || !(screen_start >= (uint8_t**) ONE_TWENTY_EIGHT_MB && screen_start <= (uint8_t**) ONE_THIRTY_TWO_MB)) {
        return -1;
    }
    else {
        page_directory[USER_ADDR_INDEX + 1].kb.page_size = 0;   // 4 kB pages
        page_directory[USER_ADDR_INDEX + 1].kb.present = 1; // set to present
        page_directory[USER_ADDR_INDEX + 1].kb.base_addr = ((unsigned int)(vid_map) >> shift_12); // physical address set
        page_directory[USER_ADDR_INDEX + 1].kb.user_supervisor = 1; //giving user access
        page_directory[USER_ADDR_INDEX + 1].kb.global = 1;
        vid_map[0].present = 1; // set to present
        vid_map[0].user_supervisor = 1; //giving user access
        vid_map[0].base_addr = (int) (VIDEO_ADDR / ALIGN); // set to vid mem
        flushTLB();
        *screen_start = (uint8_t*) ONE_TWENTY_EIGHT_MB + FOUR_MB; // setting start of virtual video memory
    }

    return 0;
}

/* system_set_handler(int32_t signum, void* handler_access)
 * Inputs: int32_t signum, void* handler_access
 * Return Value: 
 * Function: not implemented
 */
int32_t system_set_handler(int32_t signum, void* handler_access) {
    return -1;
}

/* system_sigreturn(void)
 * Inputs: none
 * Return Value: 
 * Function: not implemented
 */
int32_t system_sigreturn(void) {
    return -1;
}

/* process_page(int process_id)
 * Inputs: int process_id: process_id that we want to set up paging for
 * Return Value: nothing
 * Function: Makes sure process id is valid,
 * Sets page directory, at user address index, values to correct values.
 */
void process_page(int process_id) {
    // parameter checks
    if (process_id >= 0 && process_id < NUM_PROCESSES) {
        // set page directory entry
        // index will never change (virtual mem), base_addr will change (phys mem)
        page_directory[USER_ADDR_INDEX].mb.present = 1;
        page_directory[USER_ADDR_INDEX].mb.base_addr = (EIGHT_MB + FOUR_MB*process_id) >> shift_22;
        page_directory[USER_ADDR_INDEX].mb.user_supervisor = 1;
        page_directory[USER_ADDR_INDEX].mb.global = 1;
    }
}

/* get_pcb(uint32_t pid)
 * Inputs: uint32_t pid: process_id that we want to get correct pcb pointer for
 * Return Value: pcb pointer
 * Function: Calculates correct PCB pointer based on pid.
 */
pcb_t* get_pcb(uint32_t pid) {
    return (pcb_t *) (EIGHT_MB - (pid + 1) * EIGHT_KB); // pcb start address formula
}

/* update_tss(int new_pid, int terminal_id)
 * DESCRIPTION: Updates tss values of the inputted terminal
 * Inputs: int new_pid: pid used for tss_esp0 calculation, 
 *         int terminal_id: terminal index for terminal array (which terminal to alter)
 * Outputs: none
 * Return Value: none
 * Function: Used in halt and execute to alter the base tss values of each terminal
 */
void update_tss(int new_pid, int terminal_id){
    terminal_array[terminal_id].base_tss_esp0 = EIGHT_MB - new_pid * EIGHT_KB;
    terminal_array[terminal_id].base_tss_ss0 = KERNEL_DS;
}

