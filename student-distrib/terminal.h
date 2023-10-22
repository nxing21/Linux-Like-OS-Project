#include "lib.h"

#define CTL_PRESSED    0x1D
#define TERMINAL_READ  0x11
#define TERMINAL_WRITE 0x12
#define BACKSPACE_PRESSED 0x0E
#define ENTER_KEY 0x1C
#define MAX_BUF_SIZE  128
#define END_OF_LINE 0x0A

/* Initializes the cursor. */
void init_cursor();

// /* I/O control for the terminal. */
// int terminal_ioctl(uint8_t code, unsigned long arg, unsigned int bytes);

/* Prints a string of characters to the screen */
int terminal_read(uint32_t fd, void * user_buf, int count);

/* Writes data from a buffer to the screen.  */
int terminal_write(uint32_t fd ,void* user_buf, unsigned int bytes);

/* Initializes the buffer by clearing the buffer. */
int terminal_open(const char* filename);

/* Does nothing. Returns 0. */
int terminal_close(uint32_t fd);

/* Clears the terminal buffer. */
int clear_buffer();

/* Deletes a character from the screen and the input buffer. */
int edit_buffer(uint8_t response);

/* Implements control commands. */
int terminal_ctrl_cmd(unsigned long arg);
