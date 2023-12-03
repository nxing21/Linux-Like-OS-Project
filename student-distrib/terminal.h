#ifndef _TERMINAL_
#define _TERMINAL_

#include "lib.h"

#define CTL_PRESSED    0x1D
#define BACKSPACE_PRESSED 0x0E
#define ENTER_KEY 0x1C
#define MAX_BUF_SIZE  128
#define END_OF_LINE 0x0A
#define MAX_TERMINALS 3
#define CTL_L_CMD 255

//typing flag to let lib.c no to display on main video page
int DISPLAY_ON_MAIN_PAGE;

//Keeps track of Terminal being looked at (Displayed Terminal)
int screen_terminal;

/* Keeps track of the current terminal being scheduled (Terminal currently being handled by scheduler). */
int curr_terminal;

/* A struct holding information about the terminal. */
typedef struct terminal_info {
    int flag;
    int terminal_id;
    int screen_x;
    int screen_y;
    uint8_t buffer[MAX_BUF_SIZE];
    uint8_t buffer_size;
    int pid;
    uint32_t base_tss_esp0;
    uint32_t base_tss_ss0;
    int waitingInRead;
    int enter_flag;
    uint8_t attribute;
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

/* Deletes a character from the screen and the input buffer. */
int edit_buffer(uint8_t response);

/* Initialize the terminal */
void init_terminal();

#endif
