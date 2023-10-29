#include "file_sys.h"
#include "rtc.h"
#include "syscalls.h"



int32_t system_execute(const uint8_t* command) {
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
