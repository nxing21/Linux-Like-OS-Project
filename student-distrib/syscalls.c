#include "file_sys.h"
#include "rtc.h"
#include "syscalls.h"
#include "lib.h"
#include "page.h"
#include "x86_desc.h"

uint8_t cur_processes[NUM_PROCESSES] = {0,0,0,0,0,0}; // we only have two processes for checkpoint 3

int32_t system_execute(const uint8_t* command) {
    int8_t elf_check[ELF_LENGTH];
    uint8_t filename[FILENAME_LEN + 1];
    int8_t buf[EIP_CHECK];
    uint32_t pid;
    
    if (command == NULL) {
        return -1;
    }
    int i; // loop counter
    
    filename[FILENAME_LEN] = '\0';
    elf_check[DEL_INDEX] = DEL;
    elf_check[E_INDEX] = E;
    elf_check[L_INDEX] = L;
    elf_check[F_INDEX] = F;

    i = 0;
    // get the name of the executable
    while (command[i] != '\0') {
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
    read_data(dentry.inode_num, 0, (uint8_t *) buf, EIP_CHECK);
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

    // Set up paging and flush TLB
    process_page(pid);
    flushTLB();

    // User-level program loader
    read_data(dentry.inode_num, 0, (uint8_t *) VIRTUAL_ADDR, FOUR_MB);

    // Create PCB
    pcb_t *pcb = (pcb_t *) (EIGHT_MB - (pid + 1) * EIGHT_KB);
    // Initialize PCB (?)
    // pcb->file_descriptors[0] = NULL;
    // pcb->file_descriptors[1] = NULL;

    for(i = 2; i < 8; i++){
        pcb->file_descriptors[i].inode = 0;
        pcb->file_descriptors[i].file_pos = 0;
        pcb->file_descriptors[i].flags = -1;
    }

    curr_fds = pcb->file_descriptors;

    // pcb->tss.esp0 = tss.esp0;
    // pcb->tss.ss0 = tss.ss0;

    // Context switch
    tss.esp0 = EIGHT_MB - (pid) * EIGHT_KB - 4;
    tss.ss0 = KERNEL_DS;
    uint32_t eip;
    read_data(dentry.inode_num, 24, (uint8_t*)&eip, 4);
    // for (i = 0; i < 4; i++) {
    //     // eip |= (buf[24+i] << (8 * i));
    // }
    // Push IRET context to stack
    // asm volatile ("                 \n\
    //             movw %%ax, %%ds     \n\
    //             pushl %%eax         \n\
    //             pushl %%ebx         \n\
    //             pushfl              \n\
    //             pushl %%ecx         \n\
    //             pushl %%edx         \n\
    //             iret                \n\
    //             "
    //             :
    //             : "a" (USER_DS), "b" (USER_ESP), "c" (USER_CS), "d" (eip)
    //             : "memory"
    // );

    // https://wiki.osdev.org/Getting_to_Ring_3
    
    // IRET
    asm volatile("cli                        \n\
                movw $0x2B, %%ax             \n\
                movw %%ax, %%ds              \n\
                pushl %0                     \n\
                pushl %1                     \n\
                pushfl                       \n\
                popl %%ebx                   \n\
                orl $0x200, %%ebx            \n\
                pushl %%ebx                  \n\
                pushl %2                     \n\
                pushl %3                     \n\
                "
                :
                : "r" (USER_DS), "r" (USER_ESP), "r" (USER_CS), "r" (eip)
                : "eax", "ebx"
                );

    asm volatile("iret");

    return 0;
}

int32_t system_halt(uint8_t status) {
 return 0;
}

int32_t system_read (int32_t fd, void* buf, int32_t nbytes){
    if((fd >= 0 && fd < FILE_DESCRIPTOR_MAX) && curr_fds[fd].flags != -1) { 
        return curr_fds[fd].file_op_table_ptr->read(fd, buf, nbytes);

    }
    else{
        return -1;
    }
}

int32_t system_write (int32_t fd, const void* buf, int32_t nbytes){
    if((fd >= 0 && fd < FILE_DESCRIPTOR_MAX) && curr_fds[fd].flags != -1) { 
        return curr_fds[fd].file_op_table_ptr->write(fd, buf, nbytes);

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
    for(i = 2; i < 8; i++){
        if(curr_fds[i].flags == -1){
            index = i;
            break;
        }
    }

    if((read_dentry_by_name(filename, &temp_dentry) != -1) && index != -1) { //check valid name and fds not full
        file_type = temp_dentry.filetype; // 0 for user-level access to RTC, 1 for the directory, and 2 for a regular file.
        curr_fds[index].flags = 1; //marking as in use
        curr_fds[index].inode = temp_dentry.inode_num; //setting to correct inode
        curr_fds[index].file_pos = 0; // initializing position to 0

        switch (file_type)
        {
            
            case 0: /* RTC */
                dir_ops_table.open = &RTC_open;
                dir_ops_table.close = &RTC_close;
                dir_ops_table.write = &RTC_write;
                dir_ops_table.read = &RTC_read;
                curr_fds[index].file_op_table_ptr = &dir_ops_table;
                break;

            case 1: /* directory */
                dir_ops_table.open = &open_directory;
                dir_ops_table.close = &close_directory;
                dir_ops_table.read = &read_directory;
                dir_ops_table.write = &write_directory;
                curr_fds[index].file_op_table_ptr = &dir_ops_table;
                break;

            case 2: /* file */
                dir_ops_table.open = &open_file;
                dir_ops_table.close = &close_file;
                dir_ops_table.read = &read_file;
                dir_ops_table.write = &write_file;
                curr_fds[index].file_op_table_ptr = &dir_ops_table;
                break;
            
            default:
                break;
        }

        return curr_fds[index].file_op_table_ptr->open(filename);
    }
    else{
        printf("open fd not found");
        return -1;
    }
}

int32_t system_close (int32_t fd){
    //check if fd is valid index and if fd is in use
    if((fd >= 0 && fd < FILE_DESCRIPTOR_MAX) && curr_fds[fd].flags != -1) { 
        return curr_fds[fd].file_op_table_ptr->close(fd);

    }
    else{
        return -1;
    }
}

void process_page(int process_id) {
    // parameter checks
    if (process_id >= 0) {
        // set page directory entry
        // index will never change (virtual mem), base_addr will change (phys mem)
        page_directory[USER_ADDR_INDEX].mb.present = 1;
        page_directory[USER_ADDR_INDEX].mb.base_addr = (EIGHT_MB + FOUR_MB*process_id) >> shift_22;
        page_directory[USER_ADDR_INDEX].mb.user_supervisor = 1;
        // page_directory[USER_ADDR_INDEX].mb.base_addr = (EIGHT_MB + FOUR_MB*(process_id+1)) >> shift_22;
    }
}

