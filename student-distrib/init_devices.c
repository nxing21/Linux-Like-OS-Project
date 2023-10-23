#include "init_devices.h"
#include "terminal.h"

/* Global variables for handling keyboard */
static int shift_held = 0;
static int caps_locked_activated = 0;
static int caps_lock_held = 0;
static int ctrl_held = 0;

/* Making a keyboard buffer. */
uint8_t keyboard_buffer[MAX_BUFFER_SIZE];
int keyboard_buffer_size = 0;

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
    keyboard_buffer_size = 0;
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
void caps_lock_handler(uint8_t response){
    /* Cases with CAPS lock */
    if (response == CAPS_LOCK_PRESSED && caps_lock_held == 0 && caps_locked_activated == 0){
        caps_lock_held = 1;
        caps_locked_activated = 1;
    }
    if (response == CAPS_LOCK_PRESSED && caps_locked_activated == 1 && caps_lock_held == 0){
        caps_locked_activated = 0;
        caps_lock_held = 1;
    }
    if (response == CAPS_LOCK_RELEASED){
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
void shift_key_handler(uint8_t response){
    if (response == LEFT_SHIFT_PRESSED || response == RIGHT_SHIFT_PRESSED){
        shift_held = 1;
    }
    if (response == RIGHT_SHIFT_RELEASED || response == LEFT_SHIFT_RELEASED){
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
void ctrl_key_handler(uint8_t response){
    if (response == LEFT_CTL_PRESSED){
        ctrl_held = 1;
    }

    if (response == LEFT_CTL_RELEASED){
        ctrl_held = 0;
    }

    /* CTRL-L: Performs a clear screen. */
    if (response == 'l' || response == 'L'){
        clear();
        keyboard_buffer_size = 0;
        clear_buffer();
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
void typing_handler(uint8_t response){
    uint8_t printed_char; /* The character to print on the keyboard */

    /* Checks what flags have been set. */
    if (caps_locked_activated == 1 && shift_held == 0){
        if (response < SCAN_CODE_MAP_SIZE){
            printed_char = caps_lock_scan_data(response);
        }
    }
    else if (shift_held == 1 && caps_locked_activated == 0){
        if (response < SCAN_CODE_MAP_SIZE){
            printed_char = shift_key_scan_data(response);
        }
    }
    else if (shift_held && caps_locked_activated){
        if (response < SCAN_CODE_MAP_SIZE){
            printed_char = shift_and_caps_data(response);
        }
    }
    else{
        if (response < SCAN_CODE_MAP_SIZE){
            printed_char = default_scan_code_data(response);
        }   
    }
    
    /* Special case where the CTRL key is held. */
    if (ctrl_held == 1){
        ctrl_key_handler(printed_char);
        return;
    }

    /* Checks if the current ASCII can be printed. If so, add to the buffer and print to screen. */
    if (printed_char != 0x0){
        if (keyboard_buffer_size < 0){
            keyboard_buffer_size = 0;
        }
        if (keyboard_buffer_size == MAX_BUFFER_SIZE-1){
        }
        else{
            keyboard_buffer[keyboard_buffer_size] = printed_char;
            edit_buffer(printed_char);
            keyboard_buffer_size++;
            putc(printed_char);
        }
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
void backspace_handler(){
    /* If the buffer has characters in it, delete them off the screen and the buffer. */
    if (keyboard_buffer_size > 0){
        erase_char();
        keyboard_buffer[keyboard_buffer_size] = 0x0;
        keyboard_buffer_size--;
        edit_buffer(BACKSPACE_PRESSED);
    }
    if (keyboard_buffer_size == 0){
        edit_buffer(BACKSPACE_PRESSED);
        keyboard_buffer[keyboard_buffer_size] = 0x0;
    }
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
void enter_key_handler(){
    int i; 

    /* Adds the new line character to the screen.  */
    edit_buffer(ENTER_PRESESED);
    putc('\n');
    
    /* Clear out the keyboard buffer. */
    for (i = 0; i < keyboard_buffer_size +1; i++){
        keyboard_buffer[i] = 0x0;
    }
    keyboard_buffer_size = 0;
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
void tab_key_handler(){
    int i;
    /* Try to add four spaces to the buffer and to the screen. */
    for (i = 0; i < 4; i++){
         if (keyboard_buffer_size > MAX_BUF_INDEX-1){
            return;
        }
        else{
            keyboard_buffer[keyboard_buffer_size] = SPACE_ASCII;
            keyboard_buffer_size++;
            putc(SPACE_ASCII);
            edit_buffer(SPACE_ASCII);
        }
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
uint8_t default_scan_code_data(uint8_t response){
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
uint8_t caps_lock_scan_data(uint8_t response){
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
uint8_t shift_key_scan_data(uint8_t response){
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
uint8_t shift_and_caps_data(uint8_t response){
    uint8_t scan_code_data[SCAN_CODE_MAP_SIZE] = {0x0, 0x0, '!', '@', '#', '$', '%', '^', '&',
     '*' ,'(', ')', '_', '+', 0x0, 0x0, 'q','w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p','{', '}',  
    NEW_LINE_ASCII, 0x0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '"', '~', 0x0, '|', 'z', 'x', 'c', 
    'v', 'b', 'n', 'm', '<', '>', QUESTION_MARK_ASCII, 0x0, '*', 0x0, SPACE_ASCII};

    return scan_code_data[response];
}
