#include "init_devices.h"
uint8_t scan_code_data[59] = {0x0, 0x0, '1', '2', '3', '4', '5', '6', '7', '8' ,'9', '0', '-', '=', 0x0, 0x0, 'q','w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[' , ']', 0x0A, 0x0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', 0x0, '`', 0x0, 0x0, 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', 0x0, 0x0, '*', 0x0, 0x20} ;

// Function to initialize the keyboard
void init_ps2devices(){
    // uint8_t garbage;
    // uint8_t config_byte;
    // // uint8_t test_byte; /* Used for testing purposes */
    // int timer;
    // uint8_t status_byte;
    // int poll_input_status;

    // poll_input_status = 1;
    // timer = 0;

    // /* Disable the devices */
    // outb(DISABLE_PS2_PORT1, PS2_COMMAND_PORT);

    // /* Disables the second device (if not available, IO port ignores)*/
    // outb(DISABLE_PS2_PORT2, PS2_COMMAND_PORT);

    // /* Flush the output buffer */
    // garbage = inb(PS2_DATA_PORT);

    // /* Disable all IRQs and disable translation*/
    // outb(READ_CTR_CONFIG, PS2_COMMAND_PORT);
    // config_byte = inb(PS2_DATA_PORT);
    // config_byte &= 0xDC; // Clears bytes 0,1, and 6
    // outb(WRITE_PS2_DATA, PS2_COMMAND_PORT);
    // outb(config_byte, PS2_DATA_PORT); /* Write the next byte to Data port*/

    // /* Uncomment this section only to test controller and the first port (keyboard)*/
    // // outb(TEST_PS2_CONTROLLER, PS2_COMMAND_PORT); /* Test for controller*/
    // // test_byte = inb(PS2_DATA_PORT);
    // // printf("%x\n", test_byte); /* Should be 0x55 on sucess. 0xFC on fail*/

    // // outb(TEST_PS2_FIRST_PORT, PS2_COMMAND_PORT);
    // // test_byte = inb(PS2_DATA_PORT);
    // // printf("%x\n", test_byte); /* 0x00 on success, see OSDev for other response bytes*/

    // /* Enable the devices */
    // outb(ENABLE_PS2_PORT1, PS2_COMMAND_PORT);
    // // outb(ENABLE_PS2_PORT2, PS2_COMMAND_PORT); // Only use if using second PS2 port

    // /* Enable IRQs (For now, only using first device's IRQ) */
    // outb(READ_CTR_CONFIG, PS2_COMMAND_PORT);
    // config_byte = inb(PS2_DATA_PORT);
    // config_byte |= 0x01; // Sets bit 0 to 1, enabling IRQs on first PS/2 port
    // outb(WRITE_PS2_DATA, PS2_COMMAND_PORT);
    // outb(config_byte, PS2_DATA_PORT);

    // /* Reset device in first port */
    // /* Setting up a polling timer to send data */
    // while (poll_input_status == 1){
    //     if (timer == 100000){
    //         return -1;
    //     }
    //     status_byte = inb(PS2_COMMAND_PORT);
    //     poll_input_status = (status_byte >> 1) & 0x01;
    //     timer++;
    // }
    // outb(RESET_DEVICE, PS2_DATA_PORT);

    // /* Enable IRQ 1 in the PIC*/
    enable_irq(KEYBOARD_IRQ);
}

void keyboard_handler(){
    /* Check if the bit is masked. If it is, we can't deal with the interrupt
    so we return*/
    // int output_buffer_status = 0;

    uint8_t response;
    /* Gets the data from the keyboard. For now, just echo the key*/
    /* Read the data from the response byte from the keyboard */
    response = inb(PS2_DATA_PORT);
    if (response < 58){
        putc(scan_code_data[response]);
    }
    send_eoi(KEYBOARD_IRQ);

}
