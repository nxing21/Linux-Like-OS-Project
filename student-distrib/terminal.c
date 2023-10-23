/* File for the terminal driver. */
#include "lib.h"
#include "terminal.h"

uint8_t buffer[MAX_BUF_SIZE];
uint8_t write_buffer[MAX_BUF_SIZE];
int buffer_size = 0;

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
int terminal_read(uint32_t fd, void * user_buf, int count){
    int numbytes = 0; /* number of bytes read. */
    int i; /* allows us to iterate through the buffer. */
    while (buffer[buffer_size-1] != END_OF_LINE){
    }

    /* copies the terminal buffer into the userspace buffer. */
    for (i = 0; i < count; i++){
        if (i < buffer_size){
            numbytes++;
            ((uint8_t *)user_buf)[i] = buffer[i]; 
        } 
        if (buffer[i] == END_OF_LINE){
            break;
        }
    }
    buffer_size = 0;
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
int terminal_write(uint32_t fd ,void* user_buf, unsigned int bytes){
    int numbytes = 0; /* Number of bytes written. */
    int i; /* Iterates through the user buffer. */

    /* clear the write buffer. */
    for (i = 0; i < MAX_BUF_SIZE; i++){
        write_buffer[i] = 0x0;
    }
    /* Copies the user buffer into a write buffer. */
    for (i = 0; i < bytes; i++){
        write_buffer[i] = ((uint8_t *)user_buf)[i];
    }

    for (i = 0; i < bytes; i++){
        if (write_buffer[i] != NULL){
            putc(write_buffer[i]);
            numbytes++;
        }
        }

    /* clear the buffer. */
    for (i = 0; i < buffer_size; i++){
        buffer[i] = 0x0;
    }
    buffer_size = 0;

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
int edit_buffer(uint8_t response){
    cli();
        /* Case where the terminal buffer is full. */
        if (buffer_size == MAX_BUF_SIZE-2 && response == ENTER_KEY){
            buffer[buffer_size] = END_OF_LINE;
            buffer_size++;
        }
        /* Case to delete from the buffer. */
        else if (response == BACKSPACE_PRESSED){
            if (buffer_size > 0){
                buffer[buffer_size] = 0x0;
                buffer_size--;
            }
            if (buffer_size == 0){
                buffer[buffer_size] = 0x0;
            }
        }
        /* Add a character to the buffer */
        else{
            if (response == ENTER_KEY){
                buffer[buffer_size] = END_OF_LINE;
            }
            else{
                buffer[buffer_size] = response;
            }
            buffer_size++;
        }
    sti();
    return 0;
}

/* 
 * clear_buffer
 *   DESCRIPTION: Clears the input buffer and resets the size.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: Returns 0 on success.
 *   SIDE EFFECTS: Clears the input buffer and reset the size.
 */
int clear_buffer(){
    /* clear the buffer, resetes size. */
     int i; 
     for (i = 0; i < MAX_BUF_SIZE; i++){
         buffer[i] = 0x0;
         write_buffer[i] = 0x0;
    }
    buffer_size = 0;
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
int terminal_open(const char* filename){
    int i; /* allows us to iterate through the buffer. */

    /* clear the buffer. */
    for (i = 0; i < buffer_size; i++){
        buffer[i] = 0x0;
    }
    buffer_size = 0;
    return -1;
}

/* 
 * terminal_close
 *   DESCRIPTION: Closes the terminal.
 *   INPUTS: fd
 *   OUTPUTS: none
 *   RETURN VALUE: Returns 0 on success, -1 on failure.
 *   SIDE EFFECTS: None yet.
 */
int terminal_close(uint32_t fd){
    return -1;
}
