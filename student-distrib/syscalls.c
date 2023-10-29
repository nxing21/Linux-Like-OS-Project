#include "file_sys.h"
#include "rtc.h"
#include "syscalls.h"



int32_t system_execute(const uint8_t* command) {
    
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