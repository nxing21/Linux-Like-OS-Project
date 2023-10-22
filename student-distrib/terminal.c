/* File for the terminal driver. */
/* Note: Have something that keeps track of the position of the cursor. Have something that both prints out the data to the screen and prints it to the buffer*/
#include "lib.h"
#include "terminal.h"

uint8_t buffer[MAX_BUF_SIZE];
int buffer_size = 0;

// /* Initializes the cursor. */
// void init_cursor(){
//     // /* Test to disable the cursor. WORKS. This is stuff with the VGA. */
//     // outb(0x0A, 0x3D4);
//     // outb(0x20, 0x3D5);
//     /* This is the code to put the cursor in a certain position.*/
//     uint16_t position;
//     position = 7; /* Follow this formula: position = y_pos * VGA_WIDTH + x*/
//     /* I think VGA width is 80. */
//     /* Trying to figure out how to update the cursor. */

//     outb(0x0F, 0x3D4);
//     outb((uint8_t)(position && 0xFF), 0x3D5);
//     outb(0x0E,0x3D4);
//     outb((uint8_t)((position >> 8) && 0xFF), 0x3D5);
// }

// /* The ioctl for terminal. */
// int terminal_ioctl(uint8_t code, unsigned long arg, unsigned int bytes){
//     switch(code){
//         // case CTL_PRESSED:
//         //     return terminal_ctrl_cmd(arg);
//         case TERMINAL_READ:
//             // return terminal_read();
//         case TERMINAL_WRITE:
//             return terminal_write(arg, bytes);
//         case BACKSPACE_PRESSED:
//             return terminal_backspace();
//         default:
//             return -1;
//     }
// }

// /* Max characters a user can type is 127 characters (the 128th space is for \n)*/
// /* reads from terminal/data */
// int terminal_write(unsigned long arg, unsigned int bytes){
//     int success_write = 1;
//     uint8_t  * character;
//     character = (unsigned long*)(arg);
//     int i; /* loop through the string given by line_printed*/

//     for (i = 0; i < bytes; i++){
//         if (buffer_size < 0){
//             buffer_size = 0;
//         }
//         if (buffer_size == 127){
//             success_write = 0;
//         }
//         else{
//             buffer[buffer_size] = character[i];
//             buffer_size++;
//             putc(character[i]);
//             // move_cursor();
//         }
//     }
//     return success_write;

// }

/* When backspace on the keyboard is pressed, erase the character from the screen. */
// int terminal_backspace(){
//     if (buffer_size >= 0){
//         erase_char();
//         // move_cursor();
//         buffer[buffer_size] = 0x0;
//         buffer_size--;
//     }
//     return 1;
// }

/* */
// int terminal_ctrl_cmd(unsigned long arg){
//     uint8_t * command_letter;
//     command_letter = (unsigned long*)(arg);

//     switch (*command_letter){
//         case 'L':
//             clear();
//             return 1;
//         case 'l':
//             clear();
//             return 1;
//         default:
//             return -1;
//             break;
//     }
// }
/* splits up the string into fields. */

/* Copies the keyboard buffer into a userspace buffer. */
/* user_buf is the buffer to copy to. count is the number of bytes the
userspace wants to read. */
int terminal_read(uint32_t fd, void * user_buf, int count){
    int numbytes = 0; /* number of bytes read. */
    int i; /* allows us to iterate through the buffer. */
    if (buffer[buffer_size-1] != END_OF_LINE){
        return -1;
    }

    /* copies the terminal buffer into the userspace buffer. */
    for (i = 0; i < count; i++){
        if (i < buffer_size && buffer[i] != 0x0){
            numbytes++;
            ((uint8_t *)user_buf)[i] = buffer[i]; 
        } 
    }

    /* clear the buffer. */
    for (i = 0; i < buffer_size; i++){
        buffer[i] = 0x0;
    }
    buffer_size = 0;
    return numbytes;
}

/* Max characters a user can type is 127 characters (the 128th space is for \n)*/
/* reads from terminal/data */
int terminal_write(uint32_t fd ,void* user_buf, unsigned int bytes){
    int numbytes = 0; /* Number of bytes written. */
    int i; /* Iterates through the user buffer. */
    uint8_t c;
    for (i = 0; i < bytes; i++){
            c = ((uint8_t *)user_buf)[i];
            putc(((uint8_t *)user_buf)[i]);
            numbytes++;
        }
    return numbytes;

}

/* Edits the intermediate buffer between the keyboard and the terminal buffer.*/
int edit_buffer(uint8_t response){
    cli();
        if (buffer_size == MAX_BUF_SIZE-2 && response == ENTER_KEY){
            buffer[buffer_size] = END_OF_LINE;
            buffer_size++;
        }
        else if (response == BACKSPACE_PRESSED){
            if (buffer_size > 0){
                buffer[buffer_size] = 0x0;
                buffer_size--;
            }
            if (buffer_size == 0){
                buffer[buffer_size] = 0x0;
            }
        }
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

/* Clears the terminal buffer. */
int clear_buffer(){
    /* clear the buffer. */
    int i; 
    for (i = 0; i < MAX_BUF_SIZE; i++){
        buffer[i] = 0x0;
    }
    buffer_size = 0;
    return 0;
}

/* Initializes the buffer by clearing the buffer. */
int terminal_open(const char* filename){
    int i; /* allows us to iterate through the buffer. */

    /* clear the buffer. */
    for (i = 0; i < buffer_size; i++){
        buffer[i] = 0x0;
    }
    buffer_size = 0;
    return 0;
}

/* Does nothing. Returns 0. */
int terminal_close(uint32_t fd){
    return 0;
}
