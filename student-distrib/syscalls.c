#include "file_sys.h"
#include "rtc.h"
#include "syscalls.h"
#include "lib.h"
#include "page.h"
#include "x86_desc.h"
#include "terminal.h"

uint8_t cur_processes[NUM_PROCESSES] = {0,0,0,0,0,0}; // we only have two processes for checkpoint 3

void init_fops_table() {
    term_write_ops.open = NULL;
    term_write_ops.close = NULL;
    term_write_ops.write = &terminal_write;
    term_write_ops.read = NULL;

    term_read_ops.open = NULL;
    term_read_ops.close = NULL;
    term_read_ops.write = NULL;
    term_read_ops.read = &terminal_read;   
}

int32_t system_execute(const uint8_t* command) {
    int8_t elf_check[ELF_LENGTH];
    uint8_t filename[FILENAME_LEN + 1];
    int8_t buf[ELF_LENGTH];
    uint32_t pid;
    
    if (command == NULL) {
        return -1;
    }
    int i; // loop counter
    
    elf_check[DEL_INDEX] = DEL;
    elf_check[E_INDEX] = E;
    elf_check[L_INDEX] = L;
    elf_check[F_INDEX] = F;

    i = 0;
    // get the name of the executable
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
    // check the validity of the filename
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
            return -1; // no available space for new process
        }
        else if (cur_processes[i] == 0) {
            cur_processes[i] = 1;
            pid = i;
            break;
        }
    }
    curr_pid = pid;

    // Set up paging and flush TLB
    process_page(pid);
    flushTLB();

    // User-level program loader
    read_data(dentry.inode_num, 0, (uint8_t *) VIRTUAL_ADDR, FOUR_MB);

    // Create PCB
    pcb_t *pcb = get_pcb(pid);
    // Initialize PCB (?)
    pcb->pid = pid;

    pcb->file_descriptors[0].file_op_table_ptr = &term_read_ops;
    pcb->file_descriptors[0].inode = 0;
    pcb->file_descriptors[0].file_pos = 0;
    pcb->file_descriptors[0].flags = 1;


    pcb->file_descriptors[1].file_op_table_ptr = &term_write_ops;
    pcb->file_descriptors[1].inode = 0;
    pcb->file_descriptors[1].file_pos = 0;
    pcb->file_descriptors[1].flags = 1;

    for(i = FILE_DESCRIPTOR_MIN; i < FILE_DESCRIPTOR_MAX; i++){
        pcb->file_descriptors[i].inode = 0;
        pcb->file_descriptors[i].file_pos = 0;
        pcb->file_descriptors[i].flags = -1;
    }

    // curr_fds = (fd_t*)&pcb->file_descriptors;

    // pcb->tss.esp0 = tss.esp0;
    // pcb->tss.ss0 = tss.ss0;

    // Context switch
    tss.esp0 = EIGHT_MB - pid * EIGHT_KB - 4;
    tss.ss0 = KERNEL_DS;
    pcb->tss = tss;
    // pcb->ebp = something;
    // pcb->esp = something;

    uint32_t eip;
    read_data(dentry.inode_num, 24, (uint8_t*)&eip, 4);

    // https://wiki.osdev.org/Getting_to_Ring_3
    
    // IRET
    asm volatile("                           \n\
                cli                          \n\
                movw $0x2B, %%ax             \n\
                movw %%ax, %%ds              \n\
                pushl %0                     \n\
                pushl %1                     \n\
                pushfl                       \n\
                popl %%eax                   \n\
                orl $0x200, %%eax            \n\
                pushl %%eax                  \n\
                pushl %2                     \n\
                pushl %3                     \n\
                "
                :
                : "r" (USER_DS), "r" (USER_ESP), "r" (USER_CS), "r" (eip)
                : "eax"
                );

    asm volatile("iret");

    return 0;
}

// bit mask cur esp to get pcb
// from pcb get parent pid
int32_t system_halt(uint8_t status) {
    int i;

    // If currently running shell, do nothing
    if (curr_pid == 0) {
        return -1;
    }

    // Get parent PCB
    pcb_t* pcb = get_pcb(curr_pid);
    uint32_t parent_pid = pcb->parent_pid;
    pcb_t* parent_pcb = get_pcb(parent_pid);

    // Update cur_processes
    cur_processes[curr_pid] = 0;

    // Restore parent TSS
    tss = parent_pcb->tss;
    
    // Restore paging and flush TLB
    process_page(parent_pcb->pid);
    flushTLB();


    pcb->file_descriptors[0].flags = -1; //marking as not in use
    pcb->file_descriptors[0].inode = -1; //marking as not pointing to any inode
    pcb->file_descriptors[0].file_pos = 0; //file position reset to 0 

    pcb->file_descriptors[1].flags = -1; //marking as not in use
    pcb->file_descriptors[1].inode = -1; //marking as not pointing to any inode
    pcb->file_descriptors[1].file_pos = 0; //file position reset to 0 

    // Close all file operations
    for (i = 2; i < FILE_DESCRIPTOR_MAX; i++) {
        system_close(i);
    }

    // assembly to load old esp, ebp and load 
    asm volatile("                           \n\
                movl %0, %%ebp               \n\
                movl %1, %%esp               \n\
                movl %2, %%eax               \n\
                "
                :
                : "r" (parent_pcb->ebp), "r" (parent_pcb->esp), "r" ((uint32_t) status)
                : "memory"
                );

    return status;
}

int32_t system_read (int32_t fd, void* buf, int32_t nbytes){
    pcb_t *pcb = get_pcb(curr_pid);
    if((fd >= 0 && fd < FILE_DESCRIPTOR_MAX) &&   pcb->file_descriptors[fd].flags != -1) { 
        return pcb->file_descriptors[fd].file_op_table_ptr->read(fd, buf, nbytes);
    }
    else{
        return -1;
    }
}

int32_t system_write (int32_t fd, const void* buf, int32_t nbytes){
    pcb_t *pcb = get_pcb(curr_pid);
    if((fd >= 0 && fd < FILE_DESCRIPTOR_MAX) && pcb->file_descriptors[fd].flags != -1) { 
        return pcb->file_descriptors[fd].file_op_table_ptr->write(fd, buf, nbytes);
    }
    else{
        return -1;
    }
}

int32_t system_open (const uint8_t* filename){
    dentry_t temp_dentry;
    uint32_t file_type;
    int i;
    int index = -1;
    pcb_t *pcb = get_pcb(curr_pid);
    for(i = FILE_DESCRIPTOR_MIN; i < FILE_DESCRIPTOR_MAX; i++){
        if(pcb->file_descriptors[i].flags == -1){
            index = i;
            break;
        }
    }

    if((read_dentry_by_name(filename, &temp_dentry) != -1) && index != -1) { //check valid name and fds not full
        file_type = temp_dentry.filetype; // 0 for user-level access to RTC, 1 for the directory, and 2 for a regular file.
        pcb->file_descriptors[index].flags = 1; //marking as in use
        pcb->file_descriptors[index].inode = temp_dentry.inode_num; //setting to correct inode
        pcb->file_descriptors[index].file_pos = 0; // initializing position to 0

        switch (file_type)
        {
            case 0: /* RTC */
                dir_ops_table.open = &RTC_open;
                dir_ops_table.close = &RTC_close;
                dir_ops_table.write = &RTC_write;
                dir_ops_table.read = &RTC_read;
                pcb->file_descriptors[index].file_op_table_ptr = &dir_ops_table;
                break;

            case 1: /* directory */
                dir_ops_table.open = &open_directory;
                dir_ops_table.close = &close_directory;
                dir_ops_table.read = &read_directory;
                dir_ops_table.write = &write_directory;
                pcb->file_descriptors[index].file_op_table_ptr = &dir_ops_table;
                break;

            case 2: /* file */
                dir_ops_table.open = &open_file;
                dir_ops_table.close = &close_file;
                dir_ops_table.read = &read_file;
                dir_ops_table.write = &write_file;
                pcb->file_descriptors[index].file_op_table_ptr = &dir_ops_table;
                break;
            
            default:
                break;
        }

        return index;
    }
    else{
        // printf("open fd not found");
        return -1;
    }
}

int32_t system_close (int32_t fd){
    pcb_t *pcb = get_pcb(curr_pid);
    //check if fd is valid index and if fd is in use
    if((fd >= 0 && fd < FILE_DESCRIPTOR_MAX) && pcb->file_descriptors[fd].flags != -1) { 
        pcb->file_descriptors[fd].flags = -1; //marking as not in use
        pcb->file_descriptors[fd].inode = -1; //marking as not pointing to any inode
        pcb->file_descriptors[fd].file_pos = 0; //file position reset to 0 
        return pcb->file_descriptors[fd].file_op_table_ptr->close(fd);
    }
    else{
        return -1;
    }
}

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

// void delete_page(int process_id) {
//     // parameter checks
//     if (process_id >= 0 && process_id < NUM_PROCESSES) {
//         // set page directory entry
//         // index will never change (virtual mem), base_addr will change (phys mem)
//         page_directory[USER_ADDR_INDEX].mb.present = 0;
//         page_directory[USER_ADDR_INDEX].mb.base_addr = 0;
//         page_directory[USER_ADDR_INDEX].mb.user_supervisor = 0;
//         page_directory[USER_ADDR_INDEX].mb.global = 0;
//     }
// }

pcb_t* get_pcb(uint32_t pid){
    return (pcb_t *) (EIGHT_MB - (pid + 1) * EIGHT_KB);
}

