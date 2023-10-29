#ifndef _SYS_CALLS_
#define _SYS_CALLS_

#include "types.h"

#define FILE_DESCRIPTOR_MAX 8
#define FILE_DESCRIPTOR_MIN 2

int32_t system_execute(const uint8_t* command);
int32_t system_halt(uint8_t status);
int32_t system_read (int32_t fd, void* buf, int32_t nbytes);
int32_t system_write (int32_t fd, const void* buf, int32_t nbytes);
int32_t system_open (const uint8_t* filename);
int32_t system_close (int32_t fd);

typedef struct  file_op_table {
    int32_t (*open)(const uint8_t* filename);
    int32_t (*close)(int32_t fd);
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
} fops_t;

typedef struct  file_descriptor {
    fops_t *file_op_table_ptr; /* The file operations jump table associated with the correct file type */
    int32_t inode; /*The inode number for this file. This is only valid for data files, and should be 0 for directories and the RTC device file.*/
    int32_t file_pos; /* keeps track of where the user is currently reading from in the file. Every read system call should update this member.*/
    int32_t flags; /* among other things, marking this file descriptor as “in-use.” */
} fd_t;

fd_t *curr_fds;



typedef struct proccess_control_block {
    fd_t file_descriptors[FILE_DESCRIPTOR_MAX];


} pcb_t;



#endif
