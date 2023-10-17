/* File for the terminal driver. */
/* Note: Have something that keeps track of the position of the cursor. Have something that both prints out the data to the screen and prints it to the buffer*/
#include "lib.h"

/* Initializes the cursor. */
void init_cursor(){
    // /* Test to disable the cursor. WORKS. This is stuff with the VGA. */
    // outb(0x0A, 0x3D4);
    // outb(0x20, 0x3D5);
    /* This is the code to put the cursor in a certain position.*/
    uint16_t position;
    position = 7; /* Follow this formula: position = y_pos * VGA_WIDTH + x*/
    /* I think VGA width is 80. */

    outb(0x0F, 0x3D4);
    outb((uint8_t)(position && 0xFF), 0x3D5);
    outb(0x0E,0x3D4);
    outb((uint8_t)((position >> 8) && 0xFF), 0x3D5);
}