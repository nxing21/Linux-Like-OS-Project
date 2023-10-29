#include "file_sys.h"
#include "rtc.h"
#include "syscalls.h"
#include "lib.h"

uint8_t cur_processes[NUM_PROCESSES] = {0,0}; // we only have two processes for checkpoint 3

int32_t system_execute(const uint8_t* command) {
    uint8_t elf_check[ELF_LENGTH];
    uint8_t filename[FILENAME_LEN + 1];
    uint8_t buf[ELF_LENGTH];
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

    dentry_t* dentry;
    // check the validity of the filename
    if (read_dentry_by_name(filename, dentry) == -1) {
        return -1;
    }

    // Check if valid executable
    read_data(dentry->inode_num, 0, buf, ELF_LENGTH);
    if (strncmp(elf_check, buf, ELF_LENGTH) != 0) {
        return -1;
    }

    // Set up paging

    // Create PCB
    // First find free PCB location
    for (i = 0; i <= NUM_PROCESSES; i++) {
        if (i == NUM_PROCESSES) {
            return -1; // no available space for new process
        }
        else if (cur_processes[i] == 0) {
            cur_processes[i] = 1;
            pid = i;
        }
    }

    pcb_t *pcb = (pcb_t *) (EIGHT_MB - pid * EIGHT_KB);
}

int32_t system_halt(uint8_t status) {

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
                curr_fds[index].file_op_table_ptr->open = &RTC_open;
                curr_fds[index].file_op_table_ptr->close = &RTC_close;
                curr_fds[index].file_op_table_ptr->read = &RTC_read;
                curr_fds[index].file_op_table_ptr->write = &RTC_write;
                break;

            case 1: /* directory */
                curr_fds[index].file_op_table_ptr->open = &open_directory;
                curr_fds[index].file_op_table_ptr->close = &close_directory;
                curr_fds[index].file_op_table_ptr->read = &read_directory;
                curr_fds[index].file_op_table_ptr->write = &write_directory;
                break;

            case 2: /* file */
                curr_fds[index].file_op_table_ptr->open = &open_file;
                curr_fds[index].file_op_table_ptr->close = &close_file;
                curr_fds[index].file_op_table_ptr->read = &read_file;
                curr_fds[index].file_op_table_ptr->write = &write_file;
                break;
            
            default:
                break;
        }

        return curr_fds[index].file_op_table_ptr->open(filename);
    }
    else{
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
