#ifndef _FILE_SYS_H_
#define _FILE_SYS_H_

#include "types.h"

#define MAX_FILES 62
#define FILENAME_LEN 32 /* 32 bytes/characters */
#define ONE_KB 1024
#define DENTRY_RESERVED_BYTES 24
#define BOOT_BLOCK_RESERVED_BYTES 52
#define DIR_ENTRIES 63
#define BYTES_PER_BLOCK 4096

/* file directory entry structure from slides*/
typedef struct dentry {
    uint8_t filename[FILENAME_LEN];
    uint32_t filetype; //File types are 0 for user-level access to RTC, 1 for the directory, and 2 for a regular file.
    uint32_t inode_num; // The index node number is only meaningful for regular files and should be ignored for the RTC and directory types.
    uint8_t reserved[DENTRY_RESERVED_BYTES]; 
} dentry_t __attribute__((packed));

/* inode structure from slides*/
typedef struct  inode {
    uint32_t length;
    uint32_t data_block_num[ONE_KB-1];
} inode_t __attribute__((packed));

/* inode structure from slides*/
typedef struct  boot_block {
    uint32_t dir_count;
    uint32_t inode_count;
    uint32_t data_count;
    uint8_t reserved[BOOT_BLOCK_RESERVED_BYTES];
    dentry_t direntries[DIR_ENTRIES];
} boot_block_t __attribute__((packed));

typedef struct  file_descriptor {
    int32_t *file_op_table_ptr; /* The file operations jump table associated with the correct file type */
    int32_t inode; /*The inode number for this file. This is only valid for data files, and should be 0 for directories and the RTC device file.*/
    int32_t file_pos; /* keeps track of where the user is currently reading from in the file. Every read system call should update this member.*/
    int32_t flags; /* among other things, marking this file descriptor as “in-use.” */
} fd_t __attribute__((packed));



// boot_block_t *boot_block;
// inode_t *inode_start;
// uint8_t *data_block_ptr;

// inode_t * inode_start;
// uint16_t *data_block_ptr;

// typedef struct file_system {
//     boot_block_t *boot_block;
//     inode_t *inode_start;
//     uint8_t *data_block_ptr;
// } file_system_t;

// static file_system_t *file_system;

void init_file_sys(uint32_t starting_addr);
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

extern int32_t read_file(int32_t fd, void* buf, int32_t nbytes);
extern int32_t write_file(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t open_file(const uint8_t* filename);
extern int32_t close_file(int32_t fd);

extern int32_t read_directory(int32_t fd, void* buf, int32_t nbytes);
extern int32_t write_directory(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t open_directory(const uint8_t* filename);
extern int32_t close_directory(int32_t fd);

#endif
