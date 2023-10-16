#include "init_devices.h"

/* Maps data from PS/2 keyboard to ASCII characters */
uint8_t scan_code_data[59] = {0x0, 0x0, '1', '2', '3', '4', '5', '6', '7', '8' ,'9', '0', '-', '=', 0x0, 0x0, 'q','w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[' , ']', 0x0A, 0x0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', 0x0, '`', 0x0, 0x0, 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', 0x0, 0x0, '*', 0x0, 0x20} ;

/* 
 * init_ps2devices
 *   DESCRIPTION: Initializes the PS/2 devices (keyboard). Done by enabling IRQ2 on the Master PIC.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Enables keyboard interrupts on the PIC
 */
void init_ps2devices(){
    /* Enable IRQ 1 in the PIC*/
    enable_irq(KEYBOARD_IRQ);
}


/* 
 * keyboard_handler
 *   DESCRIPTION: Handles any interrupts that the keyboard sends. Sends an EOI after completing the interrupt action.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Prints a character on the screen based on the scan code
 *                 sent by the PS/2 keyboard.
 */
void keyboard_handler(){
    uint8_t response;
    /* Gets the data from the keyboard. For now, just echo the key*/
    /* Read the data from the response byte from the keyboard */
    response = inb(PS2_DATA_PORT);
    if (response < 58){
        putc(scan_code_data[response]);
    }
    send_eoi(KEYBOARD_IRQ);

}
