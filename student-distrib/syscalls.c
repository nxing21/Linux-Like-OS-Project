#include "file_sys.h"
#include "syscalls.h"
#include "lib.h"

int32_t system_execute(const uint8_t* command) {
    uint8_t filename[FILENAME_LEN + 1];
    if (command == NULL) {
        return -1;
    }
    filename[FILENAME_LEN] = '\0';
    int i; // loop counter

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

    // Set up paging

    // Flush TLB
    asm volatile ();
}

int32_t system_halt(uint8_t status) {

}


int32_t read (int32_t fd, void* buf, int32_t nbytes) {

}
int32_t write (int32_t fd, const void* buf, int32_t nbytes) {

}
int32_t open (const uint8_t* filename) {
    dentry_t temp_dentry;
    uint32_t file_type;

    if((read_dentry_by_name(filename, &temp_dentry) != -1) && /* check if open fd*/) { //check valid name and fds not full
        file_type = temp_dentry.filetype; // 0 for user-level access to RTC, 1 for the directory, and 2 for a regular file.
        file_descriptors[/* index*/].flags = 1; //marking as in use
        file_descriptors[/* index*/].inode = temp_dentry.inode_num; //setting to correct inode
        fds_in_use++; // updating file descriptors in use
        return 0;
    }
    else{
        return -1;
    }
}
int32_t close (int32_t fd) {

}
