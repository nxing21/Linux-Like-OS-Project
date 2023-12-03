#include "keyboard.h"
#include "terminal.h"
#include "pit.h"
#include "syscalls.h"
#include "page.h"

/* Global variables for handling keyboard */
static int shift_held = 0;
static int caps_locked_activated = 0;
static int caps_lock_held = 0;
static int ctrl_held = 0;
static int alt_held = 0;

/* Terminal Flag used for move_cursor. */
extern int terminal_flag;

/* 
 * init_ps2devices
 *   DESCRIPTION: Initializes the PS/2 devices (keyboard). Done by enabling IRQ2 on the Master PIC.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Enables keyboard interrupts on the PIC
 */
void init_ps2devices() {
    /* Enable IRQ 1 in the PIC*/
    enable_irq(KEYBOARD_IRQ);
    DISPLAY_ON_MAIN_PAGE = 0;
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
void keyboard_handler() {
    uint8_t response; /* Scan code from the keyboard*/
    /* Gets the data from the keyboard.*/
    response = inb(PS2_DATA_PORT);

    switch(response){
        case CAPS_LOCK_PRESSED:
            caps_lock_handler(CAPS_LOCK_PRESSED);
            break;
        case CAPS_LOCK_RELEASED:
            caps_lock_handler(CAPS_LOCK_RELEASED);
            break;
        case LEFT_SHIFT_PRESSED:
            shift_key_handler(LEFT_SHIFT_PRESSED);
            break;
        case RIGHT_SHIFT_PRESSED:
            shift_key_handler(RIGHT_SHIFT_PRESSED);
            break;
        case LEFT_SHIFT_RELEASED:
            shift_key_handler(LEFT_SHIFT_RELEASED);
            break;
        case RIGHT_SHIFT_RELEASED:
            shift_key_handler(RIGHT_SHIFT_RELEASED);
            break;
        case LEFT_CTL_PRESSED:
            ctrl_key_handler(LEFT_CTL_PRESSED);
            break;
        case LEFT_CTL_RELEASED:
            ctrl_key_handler(LEFT_CTL_RELEASED);
            break;
        case BACKSPACE_PRESSED:
            backspace_handler();
            break;
        case ENTER_PRESESED:
            enter_key_handler();
            break;
        case TAB_PRESSED:
            tab_key_handler();
            break;
        case ALT_PRESSED:
            alt_key_handler(ALT_PRESSED);
            break;
        case ALT_RELEASED:
            alt_key_handler(ALT_RELEASED);
            break;
        default:
            typing_handler(response);
            break;
    }

    send_eoi(KEYBOARD_IRQ);
}

/* 
 * caps_lock_handler
 *   DESCRIPTION: Allows usage of the CAPS LOCK on the keyboard.
 *   INPUTS: uint8_t response -- Can either be CAPS LOCK PRESSED or CAPS_LOCK_RELEASED
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Sets flags based on the state of CAPS LOCK key. 
 *                 
 */
void caps_lock_handler(uint8_t response) {
    /* Cases with CAPS lock */
    if (response == CAPS_LOCK_PRESSED && caps_lock_held == 0 && caps_locked_activated == 0) {
        caps_lock_held = 1;
        caps_locked_activated = 1;
    }
    if (response == CAPS_LOCK_PRESSED && caps_locked_activated == 1 && caps_lock_held == 0) {
        caps_locked_activated = 0;
        caps_lock_held = 1;
    }
    if (response == CAPS_LOCK_RELEASED) {
        caps_lock_held = 0;
    }
}

/* 
 * shift_key_handler
 *   DESCRIPTION: Allows usage of both SHIFT keys on the keyboard.
 *   INPUTS: uint8_t response -- Accounts for the press and release of both SHIFT keys.
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Sets flags based on the state of both shift keys. 
 *                 
 */
void shift_key_handler(uint8_t response) {
    if (response == LEFT_SHIFT_PRESSED || response == RIGHT_SHIFT_PRESSED) {
        shift_held = 1;
    }
    if (response == RIGHT_SHIFT_RELEASED || response == LEFT_SHIFT_RELEASED) {
        shift_held = 0;
    }
}

/* 
 * ctrl_key_handler
 *   DESCRIPTION: Allows usage of both CTRL keys on the keyboard.
 *   INPUTS: uint8_t response -- Accounts for the press and release of both CTRL keys.
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Sets flags based on the state of both CTRL keys. 
 *                 
 */
void ctrl_key_handler(uint8_t response) {
    if (response == LEFT_CTL_PRESSED) {
        ctrl_held = 1;
    }

    if (response == LEFT_CTL_RELEASED) {
        ctrl_held = 0;
    }

    /* CTRL-L: Performs a clear screen. */
    if (response == 'l' || response == 'L') {
        edit_buffer(CTL_L_CMD);
    }
}

/* 
 * alt_key_handler
 *   DESCRIPTION: Takes care of the ALT key.
 *   INPUTS: uint8_t response -- The ALT key, the F1, F2, or F3 key.
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Sets flags based on the state of the ALT key.
 *                 In the case of terminal switching, calls a function called switch_screen whose argument is the 
 *                 terminal that the user wants to switch to. 
 *                 
 */
void alt_key_handler(uint8_t response) {
    if (response == ALT_PRESSED && alt_held == 0) {
        alt_held = 1;
    }

    if (response == ALT_RELEASED && alt_held == 1) {
        alt_held = 0;
    }

    /* Set up the logic between switching the terminals. */
    if (response == F1_PRESSED) {
        switch_screen(TERMINAL_0);
    }
    if (response == F2_PRESSED) {
        switch_screen(TERMINAL_1);
    }
    if (response == F3_PRESSED) {
        switch_screen(TERMINAL_2);
    }
}

/* 
 * typing_handler
 *   DESCRIPTION: Handles typing based on the scan code given by the keyboard.
 *   INPUTS: uint8_t response -- The scan code to map.
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Tries to add a character to the buffer and the screen. 
 *                 
 */
void typing_handler(uint8_t response) {
    uint8_t printed_char; /* The character to print on the keyboard */

    /* Checks what flags have been set. */
    if (caps_locked_activated == 1 && shift_held == 0) {
        if (response < SCAN_CODE_MAP_SIZE) {
            printed_char = caps_lock_scan_data(response);
        }
    }
    else if (shift_held == 1 && caps_locked_activated == 0) {
        if (response < SCAN_CODE_MAP_SIZE) {
            printed_char = shift_key_scan_data(response);
        }
    }
    else if (shift_held && caps_locked_activated) {
        if (response < SCAN_CODE_MAP_SIZE) {
            printed_char = shift_and_caps_data(response);
        }
    }
    else {
        if (response < SCAN_CODE_MAP_SIZE) {
            printed_char = default_scan_code_data(response);
        }   
    }
    
    /* Special case where the CTRL key is held. */
    if (ctrl_held == 1) {
        ctrl_key_handler(printed_char);
        return;
    }

    /* Special case where the alt key is held. */
    if (alt_held == 1) {
        alt_key_handler(response);
        return;
    }

    /* Checks if the current ASCII can be printed. If so, add to the buffer and print to screen. */
    if (printed_char != 0x0) {
            edit_buffer(printed_char);
    }
}

/* 
 * backspace_handler
 *   DESCRIPTION: Takes care of the BACKSPACE key.
 *   INPUTS: uint8_t response -- The backspace key.
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Tries to remove a character from the screen and the buffer.
 *                 
 */
void backspace_handler() {
    edit_buffer(BACKSPACE_PRESSED);
}

/* 
 * enter_key_handler
 *   DESCRIPTION: Takes care of the ENTER key.
 *   INPUTS: uint8_t response -- The ENTER key.
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Prints a new line on the screen, clears the keyboard buffer.
 *                 
 */
void enter_key_handler() {
    edit_buffer(ENTER_PRESESED);
}

/* 
 * tab_key_handler
 *   DESCRIPTION: Takes care of the TAB key.
 *   INPUTS: uint8_t response -- The TAB key.
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Tries to add four spaces to the buffer and to the screen.
 *                 
 */
void tab_key_handler() {
    int i;
    /* Try to add four spaces to the buffer and to the screen. */
    for (i = 0; i < 4; i++) {
        edit_buffer(SPACE_ASCII);
    } 
}

/* 
 * default_scan_code_data
 *   DESCRIPTION: Maps a scan code to an ASCII character when no flags are set.
 *   INPUTS: uint8_t response -- The scan code to map.
 *   OUTPUTS: none
 *   RETURN VALUE: The ASCII that the scan code was mapped to.
 *   SIDE EFFECTS: The keyboard handler will try to add the ASCII to the keyboard and to the screen.
 *                 
 */
uint8_t default_scan_code_data(uint8_t response) {
    uint8_t scan_code_data[SCAN_CODE_MAP_SIZE] = {0x0, 0x0, '1', '2', '3', '4', '5', '6', '7', 
    '8' ,'9', '0', '-', '=', 0x0, 0x0, 'q','w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[' , ']', 
    NEW_LINE_ASCII, 0x0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', SINGLE_QUOTE_ASCII, '`', 0x0, BACKWARD_SLASH_ASCII, 'z', 'x', 'c', 
    'v', 'b', 'n', 'm', ',', '.', FORWARD_SLASH_ASCII, 0x0, '*', 0x0, SPACE_ASCII};

    return scan_code_data[response];
}


/* 
 * caps_lock_scan_data
 *   DESCRIPTION: Maps a scan code to an ASCII character when CAPS LOCK flag is set.
 *   INPUTS: uint8_t response -- The scan code to map.
 *   OUTPUTS: none
 *   RETURN VALUE: The ASCII that the scan code was mapped to.
 *   SIDE EFFECTS: The keyboard handler will try to add the ASCII to the keyboard and to the screen.
 *                 
 */
uint8_t caps_lock_scan_data(uint8_t response) {
    uint8_t scan_code_data[SCAN_CODE_MAP_SIZE] = {0x0, 0x0, '1', '2', '3', '4', '5', '6', '7',
    '8' ,'9', '0', '-', '=', 0x0, 0x0, 'Q','W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', 
    NEW_LINE_ASCII, 0x0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', SINGLE_QUOTE_ASCII, '`', 0x0, BACKWARD_SLASH_ASCII, 'Z', 'X', 'C', 
    'V', 'B', 'N', 'M', ',', '.', FORWARD_SLASH_ASCII, 0x0, '*', 0x0, SPACE_ASCII};

    return scan_code_data[response];
}

/* 
 * shift_key_scan_data
 *   DESCRIPTION: Maps a scan code to an ASCII character when SHIFT flag is set.
 *   INPUTS: uint8_t response -- The scan code to map.
 *   OUTPUTS: none
 *   RETURN VALUE: The ASCII that the scan code was mapped to.
 *   SIDE EFFECTS: The keyboard handler will try to add the ASCII to the keyboard and to the screen.
 *                 
 */
uint8_t shift_key_scan_data(uint8_t response) {
    uint8_t scan_code_data[SCAN_CODE_MAP_SIZE] = {0x0, 0x0, '!', '@', '#', '$', '%', '^', '&',
    '*' ,'(', ')', '_', '+', 0x0, 0x0, 'Q','W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 
    NEW_LINE_ASCII, 0x0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0x0, '|', 'Z', 'X', 'C', 
    'V', 'B', 'N', 'M', '<', '>', QUESTION_MARK_ASCII, 0x0, '*', 0x0, SPACE_ASCII};

    return scan_code_data[response];
}

/* 
 * shift_and_caps_data
 *   DESCRIPTION: Maps a scan code to an ASCII character when SHIFT and CAPS LOCK flag is set.
 *   INPUTS: uint8_t response -- The scan code to map.
 *   OUTPUTS: none
 *   RETURN VALUE: The ASCII that the scan code was mapped to.
 *   SIDE EFFECTS: The keyboard handler will try to add the ASCII to the keyboard and to the screen.
 *                 
 */
uint8_t shift_and_caps_data(uint8_t response) {
    uint8_t scan_code_data[SCAN_CODE_MAP_SIZE] = {0x0, 0x0, '!', '@', '#', '$', '%', '^', '&',
     '*' ,'(', ')', '_', '+', 0x0, 0x0, 'q','w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p','{', '}',  
    NEW_LINE_ASCII, 0x0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '"', '~', 0x0, '|', 'z', 'x', 'c', 
    'v', 'b', 'n', 'm', '<', '>', QUESTION_MARK_ASCII, 0x0, '*', 0x0, SPACE_ASCII};

    return scan_code_data[response];
}

/* 
 * switch_screen
 *   DESCRIPTION: Remaps video memory corresponding to the terminal that the user wants to switch to, and saves our old video memory into a backup page.
 *   INPUTS: uint8_t new_terminal - The terminal that the user wants to switch to.
 *   OUTPUTS: none
 *   RETURN VALUE: None
 *   SIDE EFFECTS: Remaps video memory corresponding the terminal that the user wants to switch to, and repositions the cursor.
 *                 
 */
void switch_screen(uint8_t new_terminal) {
    cli();
    memcpy((char *) VIDEO_ADDR + ((screen_terminal+1) * ALIGN), (char *) VIDEO_ADDR , FOUR_KB); // save current screen mem values to backup terminal video page
    vid_map[0].base_addr = (int) (VIDEO_ADDR / ALIGN) + (screen_terminal+1); // switch user vid map to point to backup terminal page
    flushTLB();
    memcpy((char *) VIDEO_ADDR, (char *) VIDEO_ADDR + ((new_terminal+1) * ALIGN), FOUR_KB); // save backup terminal video page to  current screen mem values
    terminal_flag = 0;
    screen_terminal = new_terminal;
    move_cursor();
    sti();
}
