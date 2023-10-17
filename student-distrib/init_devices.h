#ifndef _INIT_DEVICES_H
#define _INIT_DEVICES_H

#include "lib.h"
#include "i8259.h"

/* PS/2 Controller Ports/ Commands*/
#define PS2_COMMAND_PORT    0x64 /* Command is sent */
#define PS2_DATA_PORT   0x60 /* Response byte is sent to this port, also inputs to the devices are sent to this port*/
#define READ_CTR_CONFIG 0x20
#define DISABLE_PS2_PORT1 0xAD
#define DISABLE_PS2_PORT2 0xA7
#define WRITE_PS2_DATA  0x60
#define TEST_PS2_CONTROLLER 0xAA
#define TEST_PS2_FIRST_PORT 0xAB
#define ENABLE_PS2_PORT1 0xAE
#define ENABLE_PS2_PORT2 0xA8

/* PS/2 Keyboard Commands */
#define RESET_DEVICE 0xFF

/* Mapping the IRQs to devices */
#define KEYBOARD_IRQ  1
#define SCAN_CODE_MAP_SIZE 59

/* Keyboard constants */
#define CAPS_LOCK_PRESSED 0x3A
#define CAPS_LOCK_RELEASED 0xBA

#define LEFT_SHIFT_PRESSED 0x2A
#define RIGHT_SHIFT_PRESSED 0x36
#define LEFT_SHIFT_RELEASED 0xAA
#define RIGHT_SHIFT_RELEASED 0xB6

#define READ_PS2_OUTPUT 0xD0

/* Initializes the PS/2 devices. */
void init_ps2devices();

/* Handler for keyboard interrupts. */
void keyboard_handler();

/* Takes care of cases w/ CAPS_LOCK key */
void caps_lock_handler(uint8_t response);

/* Takes care of cases w/ SHIFT key */
void shift_key_handler(uint8_t response);

/* Takes care of typing. */
void typing_handler(uint8_t response);

/* Retrieves a character based on scan code. */
uint8_t default_scan_code_data(uint8_t response);

/* Retrieves a capitalized letter based on scan code. */
uint8_t caps_lock_scan_data(uint8_t response);

/* Retrieves a SHIFT-modified character based on the scan code. */
uint8_t shift_key_scan_data(uint8_t response);

/* Maps a scan code to a character if both SHIFT and CAPS Lock is on*/
uint8_t shift_and_caps_data(uint8_t response);


#endif
