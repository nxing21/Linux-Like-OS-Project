#ifndef _TERMINAL_
#define _TERMINAL_

#include "lib.h"

#define CTL_PRESSED    0x1D
#define TERMINAL_READ  0x11
#define TERMINAL_WRITE 0x12
#define BACKSPACE_PRESSED 0x0E
#define ENTER_KEY 0x1C
#define MAX_BUF_SIZE  128
#define END_OF_LINE 0x0A
#define MAX_TERMINALS 3


//Keeps track of Terminal being looked at (Displayed Terminal)
int screen_terminal;

/* Keeps track of the current terminal being scheduled (Terminal currently being handled by scheduler). */
int curr_terminal;

/* A struct holding information about the terminal. */
typedef struct terminal_info {
    uint8_t buffer[MAX_BUF_SIZE];
    uint8_t write_buffer[MAX_BUF_SIZE];
    uint8_t buffer_size;
    uint8_t terminal_id;
    uint8_t flag;
    int pid;
    int screen_x;
    int screen_y;
} terminal_info_t;

/* An array to keep track of the 3 terminals. */
terminal_info_t terminal_array[MAX_TERMINALS];

/* Prints a string of characters to the screen */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);

/* Writes data from a buffer to the screen.  */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);

/* Initializes the buffer by clearing the buffer. */
int terminal_open(const char* filename);

/* Does nothing. Returns 0. */
int terminal_close(uint32_t fd);

/* Clears the write buffer. */
int clear_writebuffer();

/* Deletes a character from the screen and the input buffer. */
int edit_buffer(uint8_t response);

/* Initialize the terminal */
void init_terminal();

#endif
