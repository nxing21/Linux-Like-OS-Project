#ifndef _SYS_CALLS_
#define _SYS_CALLS_

#include "types.h"
#include "x86_desc.h"

#define FILE_DESCRIPTOR_MAX 8
#define FILE_DESCRIPTOR_MIN 2
#define ELF_LENGTH 4
#define DEL 0x7F
#define DEL_INDEX   0
#define E   0x45
#define E_INDEX     1
#define L   0x4C
#define L_INDEX     2
#define F   0x46
#define F_INDEX     3
#define EIGHT_MB    0x800000
#define FOUR_MB     0x400000
#define EIGHT_KB    0x2000
#define NUM_PROCESSES   6
#define VIRTUAL_ADDR    0x08048000
#define USER_ADDR_INDEX 32
#define USER_ESP        0x083FFFFC
#define EIP_CHECK       28
#define IN_USE          1
#define NOT_IN_USE      -1
#define EXCEPTION       255
#define ONE_TWENTY_EIGHT_MB (FOUR_MB*32)
#define ONE_THIRTY_TWO_MB (ONE_TWENTY_EIGHT_MB+FOUR_MB) 
#define BASE_SHELL 24

#define NUM_COLS    80

/* The pid corresponding to the terminal that we are currently looking at. */
uint32_t curr_pid;

int32_t system_execute(const uint8_t* command);
int32_t system_halt(uint8_t status);
int32_t system_read (int32_t fd, void* buf, int32_t nbytes);
int32_t system_write (int32_t fd, const void* buf, int32_t nbytes);
int32_t system_open (const uint8_t* filename);
int32_t system_close (int32_t fd);
int32_t system_getargs(uint8_t* buf, int32_t nbytes);
int32_t system_vidmap(uint8_t** screen_start);
int32_t system_set_handler(int32_t signum, void* handler_access);
int32_t system_sigreturn(void);

void process_page(int process_num);
void init_fops_table();

typedef struct file_op_table {
    int32_t (*open)(const uint8_t* filename);
    int32_t (*close)(int32_t fd);
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
} fops_t;

typedef struct file_descriptor {
    fops_t *file_op_table_ptr; /* The file operations jump table associated with the correct file type. */
    int32_t inode; /* The inode number for this file. This is only valid for data files, and should be 0 for directories and the RTC device file. */
    int32_t file_pos; /* Keeps track of where the user is currently reading from in the file. Every read system call should update this member. */
    int32_t flags; /* Among other things, marking this file descriptor as “in-use.” */
} fd_t;

int32_t system_read (int32_t fd, void* buf, int32_t nbytes);
int32_t system_write (int32_t fd, const void* buf, int32_t nbytes);
int32_t system_open (const uint8_t* filename);
int32_t system_close (int32_t fd);

typedef struct process_control_block {
    fd_t file_descriptors[FILE_DESCRIPTOR_MAX];
    uint32_t pid;
    uint32_t parent_pid;
    uint32_t terminal_id;
    uint32_t esp;
    uint32_t ebp;
    uint32_t eip;
    uint32_t tss_esp0;
    uint32_t tss_ss0;
    char* args; // keeps track of current arguments inputted per process
} pcb_t;

pcb_t* get_pcb(uint32_t pid);

fops_t term_write_ops;
fops_t term_read_ops;

fops_t rtc_ops;
fops_t file_ops;
fops_t dir_ops;

/* Flag that allows us to check if the PCB we are creating is for a base shell. We initially set this to 1 because the first program we always run is the base shell. */
int base_shell;

void update_tss(int new_pid, int terminal_id);

#endif
