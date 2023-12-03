/* File for the terminal driver. */
#include "lib.h"
#include "terminal.h"

/* 
 * init_terminal.
 *   DESCRIPTION: Initializing each terminal.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Resets flags, the buffer, the buffer size, and other necessary data for each terminal.
 */
void init_terminal() {
    int i; /* Loop through each terminal in the terminal array*/
    curr_terminal = 0;
    screen_terminal = 0;
    for (i = 0; i < MAX_TERMINALS; i++){
        terminal_array[i].terminal_id = i;
        terminal_array[i].buffer_size = 0;
        terminal_array[i].screen_x = 0;
        terminal_array[i].screen_y = 0;
        terminal_array[i].flag = 0;
        terminal_array[i].pid = -1;
        terminal_array[i].waitingInRead  = 0;
        terminal_array[i].enter_flag = 0;
    }
    // At the start, only the first terminal (terminal 0) will be active
    terminal_array[0].flag = 1; 

    /* Setting attributes of each terminal. */
    terminal_array[0].attribute =  0x07; //grey text on black bg
    terminal_array[1].attribute= 0x70; // reverse reverse
    terminal_array[2].attribute = 0x9F; // white text, blue bg

}

/* 
 * terminal_read
 *   DESCRIPTION: When enter is pressed, copies the terminal buffer into the userspace buffer.
 *   INPUTS: fd -- The file descriptor.
 *           user_buf -- The buffer to copy to.
 *           count -- How many bytes to read.
 *   OUTPUTS: none
 *   RETURN VALUE: numbytes -- Number of bytes actually read.
 *   SIDE EFFECTS: Copies the terminal buffer into the userspace buffer.
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes) {
    int numbytes = 0; /* number of bytes read. */
    int i; /* allows us to iterate through the buffer. */
    int enterAtEnd; /* checks if there is an enter at the end*/
    int true_bytes = nbytes;
    enterAtEnd = 0;

    /* Parameter checking.*/
    if (buf == NULL){
        return -1;
    }
    if (nbytes <= 0){
        return -1;
    }


    /* Indicates that this is a program that uses user input. */
    terminal_array[curr_terminal].waitingInRead = 1;

    while (1){
        cli();
        if (terminal_array[curr_terminal].enter_flag == 1){
            break;
        }
        sti();
    }

    if(MAX_BUF_SIZE < nbytes){
        true_bytes = MAX_BUF_SIZE;
    }
    /* copies the terminal buffer into the userspace buffer. */
    for (i = 0; i < true_bytes; i++) {
        if (i < terminal_array[curr_terminal].buffer_size) {
            numbytes++;
            ((uint8_t *)buf)[i] = terminal_array[curr_terminal].buffer[i]; 
        } 
        if (terminal_array[curr_terminal].buffer[i] == END_OF_LINE) {
            enterAtEnd = 1;
            break;
        }
    }
    /* Checks if the very last byte is the End of Line. */
    if (enterAtEnd == 0 && i < terminal_array[curr_terminal].buffer_size && ((uint8_t *)buf)[i-1] != END_OF_LINE) {
        ((uint8_t *)buf)[i-1] = END_OF_LINE;
    }

    /* clear the terminal buffer */
    for (i = 0; i < true_bytes; i++) {
        terminal_array[curr_terminal].buffer[i] = 0x0;
    }
    terminal_array[curr_terminal].buffer_size = 0;
    terminal_array[curr_terminal].waitingInRead = 0;
    terminal_array[curr_terminal].enter_flag = 0;
    sti();
    return numbytes;
}

/* 
 * terminal_write
 *   DESCRIPTION: Writes data from the userspace buffer into the output buffer.
 *   INPUTS: fd -- The file descriptor.
 *           user_buf -- The buffer to copy from.
 *           count -- How many bytes to write.
 *   OUTPUTS: none
 *   RETURN VALUE: numbytes -- Number of bytes actually written.
 *   SIDE EFFECTS: Copies the userpace buffer into the output buffer, prints the output buffer to the screen.
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes) {
    int numbytes = 0; /* Number of bytes written. */
    int i; /* Iterates through the user buffer. */

    /* Parameter checking.*/
    if (buf == NULL){
        return -1;
    }
    if (nbytes <= 0){
        return -1;
    }

    /* Prints characters from the write buffer to the screen. */
    cli();
    for (i = 0; i < nbytes; i++){
        if (((uint8_t *)buf)[i] != NULL) {
            putc(((uint8_t *)buf)[i]);
            numbytes++;
        }
    }
    sti();
    return numbytes;

}

/* 
 * edit_buffer
 *   DESCRIPTION: Edits the terminal input buffer.
 *   INPUTS: response -- Retrieved from the keyboard handler, the character to add to the terminal buffer.
 *   OUTPUTS: none
 *   RETURN VALUE: Returns 0 on success.
 *   SIDE EFFECTS: 
 */
int edit_buffer(uint8_t response) {
    cli();
    int i; /* Loops through the terminal buffer. */

    /* Clearing the screen with the CTRL-L command */
    if (response == CTL_L_CMD){
        DISPLAY_ON_MAIN_PAGE = 1;
        clear();
        for (i = 0; i < terminal_array[screen_terminal].buffer_size; i++) {
            DISPLAY_ON_MAIN_PAGE = 1;
            putc(terminal_array[screen_terminal].buffer[i] );
        }
    }
    /* Case when the user wants to edit the buffer and is currently in a program that takes user input. */
    else if (terminal_array[screen_terminal].waitingInRead == 1 && response != 0x0) {
        /* Case where the terminal buffer is almost full, just waiting for ENTER KEY. */ 
        if (terminal_array[screen_terminal].buffer_size == MAX_BUF_SIZE-2 && response == ENTER_KEY){
            terminal_array[screen_terminal].buffer[terminal_array[screen_terminal].buffer_size] = END_OF_LINE;
            DISPLAY_ON_MAIN_PAGE = 1;
            putc('\n');
            terminal_array[screen_terminal].enter_flag = 1;
            terminal_array[screen_terminal].buffer_size++;

        }
        /* Case to delete from the buffer. */
        else if (response == BACKSPACE_PRESSED) {
            if (terminal_array[screen_terminal].buffer_size > 0) {
                erase_char();
                terminal_array[screen_terminal].buffer[terminal_array[screen_terminal].buffer_size] = 0x0;
                terminal_array[screen_terminal].buffer_size--;
            }
            if (terminal_array[screen_terminal].buffer_size == 0) {
                terminal_array[screen_terminal].buffer[terminal_array[screen_terminal].buffer_size] = 0x0;
            }
        }
        /* Add a character to the buffer */
        else{
            if (terminal_array[screen_terminal].buffer_size == MAX_BUF_SIZE -1 && response != ENTER_KEY){
                /* Do nothing.*/
            }
            else if (response == ENTER_KEY) {
                terminal_array[screen_terminal].buffer[terminal_array[screen_terminal].buffer_size] = END_OF_LINE;
                DISPLAY_ON_MAIN_PAGE = 1;
                putc('\n');
                terminal_array[screen_terminal].enter_flag = 1;
                terminal_array[screen_terminal].buffer_size++;
            }
            else {
                terminal_array[screen_terminal].buffer[terminal_array[screen_terminal].buffer_size] = response;
                DISPLAY_ON_MAIN_PAGE = 1;
                putc(response);
                terminal_array[screen_terminal].buffer_size++;
            }
        }
    }
    sti();
    return 0;
}

/* 
 * terminal_open
 *   DESCRIPTION: Sets up the terminal.
 *   INPUTS: filename
 *   OUTPUTS: none
 *   RETURN VALUE: Returns 0 on success, -1 on failure.
 *   SIDE EFFECTS: Initializes the user buffer and clears the buffer_size.
 */
int terminal_open(const char* filename) {
    int i; /* allows us to iterate through the buffer. */

    /* clear the buffer. */
    for (i = 0; i < terminal_array[curr_terminal].buffer_size; i++){
        terminal_array[curr_terminal].buffer[i] = 0x0;
    }
    terminal_array[curr_terminal].buffer_size = 0;
    return 0;
}

/* 
 * terminal_close
 *   DESCRIPTION: Closes the terminal.
 *   INPUTS: fd
 *   OUTPUTS: none
 *   RETURN VALUE: Returns 0 on success, -1 on failure.
 *   SIDE EFFECTS: None yet.
 */
int terminal_close(uint32_t fd) {
    return 0;
}
