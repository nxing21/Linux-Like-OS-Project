#include "file_sys.h"
#include "rtc.h"
#include "syscalls.h"
#include "lib.h"
#include "page.h"
#include "x86_desc.h"
#include "terminal.h"

uint8_t cur_processes[NUM_PROCESSES] = {0,0,0,0,0,0}; // cur_processes keeps track of current processes that are running

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
    int8_t elf_check[ELF_LENGTH]; // holds ELF that we want to compare with
    uint8_t filename[FILENAME_LEN + 1]; // holds name of executable we want to execute
    int8_t buf[ELF_LENGTH]; // holds info
    uint32_t pid; // process ID
    
    if (command == NULL) {
        return -1;
    }
    int i; // looping variable
    
    // Puts correct ELF info inside elf_check
    elf_check[DEL_INDEX] = DEL;
    elf_check[E_INDEX] = E;
    elf_check[L_INDEX] = L;
    elf_check[F_INDEX] = F;

    i = 0;
    // Get the name of the executable
    while (command[i] != '\0' && i < FILENAME_LEN) {
        if (command[i] == ' ') {
            break;
        }
        else {
            filename[i] = command[i];
        }
        i++;
    }
    filename[i] = '\0';

    dentry_t dentry;
    // Check the validity of the filename
    if (read_dentry_by_name(filename, &dentry) == -1) {
        return -1;
    }

    // Check ELF magic constant
    read_data(dentry.inode_num, 0, (uint8_t *) buf, ELF_LENGTH);
    if (strncmp(elf_check, buf, ELF_LENGTH) != 0) {
        return -1;
    }

    // Find free PID location
    for (i = 0; i <= NUM_PROCESSES; i++) {
        if (i == NUM_PROCESSES) {
            printf("cannot open any more processes\n");
            return -1; // no available space for new process
        }
        else if (cur_processes[i] == 0) { // not in use process
            cur_processes[i] = 1;  // set to in use
            pid = i; // set pid
            break;
        }
    }

    // Create PCB
    pcb_t *pcb = get_pcb(pid);
    pcb->parent_pid = curr_pid;
    // Set curr_pid to current pid
    curr_pid = pid;

    // Set up paging and flush TLB
    process_page(pid);
    flushTLB();

    // User-level program loader
    read_data(dentry.inode_num, 0, (uint8_t *) VIRTUAL_ADDR, FOUR_MB);

    // Initialize PCB's pid
    pcb->pid = pid;

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
    for(i = FILE_DESCRIPTOR_MIN; i < FILE_DESCRIPTOR_MAX; i++){
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

    // Initialize PCB's ebp and esp
    pcb->ebp = temp_ebp;
    pcb->esp = temp_esp;

    // Reads EIP (bytes 24-27)
    uint32_t eip;
    read_data(dentry.inode_num, 24, (uint8_t*)&eip, 4); // Magic numbers: 24 is the starting index, 4 is the length of bytes 24-27

    // Initialize PCB's eip
    pcb->eip = eip;

    // https://wiki.osdev.org/Getting_to_Ring_3
    
    // IRET
    // First line "0x02B is USER_DS"
    // Setting up stack for IRET context switch
    asm volatile("                                          \n\
                cli                                         \n\
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
    int i; // looping variable

    // Get current and parent PCB
    pcb_t* pcb = get_pcb(curr_pid);
    uint32_t parent_pid = pcb->parent_pid;
    pcb_t* parent_pcb = get_pcb(parent_pid);
    uint32_t ext_status;

    // If currently running base shell, reload
    if (curr_pid == 0) {
        printf("cannot close base shell\n");
        
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
    cur_processes[curr_pid] = 0;

    // Set the curr_pid to the parent pid.
    curr_pid = parent_pid;
    
    // Restore paging and flush TLB
    process_page(parent_pcb->pid);
    flushTLB();

    // Close all file operations
    for (i = 0; i < FILE_DESCRIPTOR_MAX; i++) {
        pcb->file_descriptors[i].flags = NOT_IN_USE; // marking as not in use
    }

    // Restoring tss
    tss.esp0 = parent_pcb->tss_esp0;
    tss.ss0 = parent_pcb->tss_ss0;

    if(status == EXCEPTION){ // accounting for status being 8 bits
        ext_status = EXCEPTION+1;
    }
    else{
        ext_status = status;
    }
    
    // Assembly to load old esp, ebp, and status 
    asm volatile("                           \n\
                movl %0, %%eax               \n\
                movl %1, %%esp               \n\
                movl %2, %%ebp               \n\
                jmp execute_return           \n\
                "
                :
                : "r" (ext_status), "r" (pcb->esp), "r" (pcb->ebp)
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
int32_t system_read (int32_t fd, void* buf, int32_t nbytes){
    pcb_t *pcb = get_pcb(curr_pid); // getting current pcb pointer
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
int32_t system_write (int32_t fd, const void* buf, int32_t nbytes){
    pcb_t *pcb = get_pcb(curr_pid); // getting current pcb pointer
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
int32_t system_open (const uint8_t* filename){
    dentry_t temp_dentry;
    uint32_t file_type;
    int i;
    int index = -1;
    pcb_t *pcb = get_pcb(curr_pid); // getting current pcb pointer
    for(i = FILE_DESCRIPTOR_MIN; i < FILE_DESCRIPTOR_MAX; i++){ // finding first open file descriptor
        if(pcb->file_descriptors[i].flags == NOT_IN_USE){
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
                dir_ops_table.open = &RTC_open;
                dir_ops_table.close = &RTC_close;
                dir_ops_table.write = &RTC_write;
                dir_ops_table.read = &RTC_read;
                pcb->file_descriptors[index].file_op_table_ptr = &dir_ops_table;
                break;

            case 1: /* setting directory functions */
                dir_ops_table.open = &open_directory;
                dir_ops_table.close = &close_directory;
                dir_ops_table.read = &read_directory;
                dir_ops_table.write = &write_directory;
                pcb->file_descriptors[index].file_op_table_ptr = &dir_ops_table;
                break;

            case 2: /*setting file functions*/
                dir_ops_table.open = &open_file;
                dir_ops_table.close = &close_file;
                dir_ops_table.read = &read_file;
                dir_ops_table.write = &write_file;
                pcb->file_descriptors[index].file_op_table_ptr = &dir_ops_table;
                break;
            
            default:
                break;
        }

        return index; // returning fd index of opened file descriptor
    }
    else{
        return -1;
    }
}

/* system_close (int32_t fd)
 * Inputs: int32_t fd: file descriptor index.
 * Return Value: Close function result, -1 ("failure")
 * Function: Makes sure fd index and the desciptor it points to is valid,
 * if so we call the corresponding close.
 */
int32_t system_close (int32_t fd){
    pcb_t *pcb = get_pcb(curr_pid);
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
    // buf = (uint8_t*)"lol";
    return 0;
}

/* system_vidmap(uint8_t** screen_start)
 * Inputs: uint8_t** screen_start: maps the text-mode video memory into user space at a pre-set virtual address
 * Return Value: Close function result, -1 ("failure")
 * Function: Makes sure fd index and the desciptor it points to is valid,
 * if so we call the corresponding close.
 */
int32_t system_vidmap(uint8_t** screen_start) {
    return 0;
}

/* system_close (int32_t fd)
 * Inputs: int32_t fd: file descriptor index.
 * Return Value: Close function result, -1 ("failure")
 * Function: Makes sure fd index and the desciptor it points to is valid,
 * if so we call the corresponding close.
 */
int32_t system_set_handler(int32_t signum, void* handler_access) {
    return 0;
}

/* system_close (int32_t fd)
 * Inputs: int32_t fd: file descriptor index.
 * Return Value: Close function result, -1 ("failure")
 * Function: Makes sure fd index and the desciptor it points to is valid,
 * if so we call the corresponding close.
 */
int32_t system_sigreturn(void) {
    return 0;
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
pcb_t* get_pcb(uint32_t pid){
    return (pcb_t *) (EIGHT_MB - (pid + 1) * EIGHT_KB); // pcb start address formula
}

