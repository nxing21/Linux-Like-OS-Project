#include "init_devices.h"
#include "terminal.h"

/* Global variables for handling keyboard */
static int shift_held = 0;
static int caps_locked_activated = 0;
static int caps_lock_held = 0;
static int ctrl_held = 0;

/* Making a keyboard buffer. */
uint8_t keyboard_buffer[128];
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
    /* Gets the data from the keyboard. For now, just echo the key*/
    /* Read the data from the response byte from the keyboard */
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

/* Takes care of cases w/ CAPS_LOCK key */
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

/* Takes care of cases w/ SHIFT key */
void shift_key_handler(uint8_t response){
    if (response == LEFT_SHIFT_PRESSED || response == RIGHT_SHIFT_PRESSED){
        shift_held = 1;
    }
    if (response == RIGHT_SHIFT_RELEASED || response == LEFT_SHIFT_RELEASED){
        shift_held = 0;
    }
}

/* Takes care of cases w/ CTRL key */
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

    }
}

/* Takes care of typing. */
void typing_handler(uint8_t response){
    uint8_t printed_char; /* The character to print on the keyboard */

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

    if (ctrl_held == 1){
        ctrl_key_handler(printed_char);
        return;
    }

    if (printed_char != 0x0){
        if (keyboard_buffer_size < 0){
            keyboard_buffer_size = 0;
        }
        if (keyboard_buffer_size == 127){
        }
        else{
            keyboard_buffer[keyboard_buffer_size] = printed_char;
            keyboard_buffer_size++;
            putc(printed_char);
            edit_buffer(printed_char);
        }
    }
}

/* Takes care of backspace. */
void backspace_handler(){
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

/* Takes care of enter key. */
void enter_key_handler(){
    int i; 
    edit_buffer(ENTER_PRESESED);
    putc('\n');
    
    /* Clear out the keyboard buffer. */
    for (i = 0; i < keyboard_buffer_size +1; i++){
        keyboard_buffer[i] = 0x0;
    }
    keyboard_buffer_size = 0;
}

/* Takes care of the tab key. */
void tab_key_handler(){
    int i;
    for (i = 0; i < 4; i++){
         if (keyboard_buffer_size > 126){
            return;
        }
        else{
            keyboard_buffer[keyboard_buffer_size] = SPACE_ASCII;
            keyboard_buffer_size++;
            putc(SPACE_ASCII);
            edit_buffer(0x32);
        }
    }
}

/* Maps a scan code with a character.  */
uint8_t default_scan_code_data(uint8_t response){
    uint8_t scan_code_data[SCAN_CODE_MAP_SIZE] = {0x0, 0x0, '1', '2', '3', '4', '5', '6', '7', 
    '8' ,'9', '0', '-', '=', 0x0, 0x0, 'q','w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[' , ']', 
    0x0A, 0x0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', 0x27, '`', 0x0, 0x5C, 'z', 'x', 'c', 
    'v', 'b', 'n', 'm', ',', '.', 0x2F, 0x0, '*', 0x0, 0x20};

    return scan_code_data[response];
}

/* When CAPS LOCK is on, maps a scan to a capitalized letter.*/
uint8_t caps_lock_scan_data(uint8_t response){
    uint8_t scan_code_data[SCAN_CODE_MAP_SIZE] = {0x0, 0x0, '1', '2', '3', '4', '5', '6', '7',
    '8' ,'9', '0', '-', '=', 0x0, 0x0, 'Q','W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', 
    0x0A, 0x0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', 0x27, '`', 0x0, 0x5C, 'Z', 'X', 'C', 
    'V', 'B', 'N', 'M', ',', '.', 0x2F, 0x0, '*', 0x0, 0x20};

    return scan_code_data[response];
}

/* When SHIFT key is activated, maps a scan code to a SHIFT-modified key */
uint8_t shift_key_scan_data(uint8_t response){
    uint8_t scan_code_data[SCAN_CODE_MAP_SIZE] = {0x0, 0x0, '!', '@', '#', '$', '%', '^', '&',
    '*' ,'(', ')', '_', '+', 0x0, 0x0, 'Q','W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 
    0x0A, 0x0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0x0, '|', 'Z', 'X', 'C', 
    'V', 'B', 'N', 'M', '<', '>', 0x3F, 0x0, '*', 0x0, 0x20};

    return scan_code_data[response];
}

uint8_t shift_and_caps_data(uint8_t response){
    uint8_t scan_code_data[SCAN_CODE_MAP_SIZE] = {0x0, 0x0, '!', '@', '#', '$', '%', '^', '&',
     '*' ,'(', ')', '_', '+', 0x0, 0x0, 'q','w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p','{', '}',  
    0x0A, 0x0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '"', '~', 0x0, '|', 'z', 'x', 'c', 
    'v', 'b', 'n', 'm', '<', '>', 0x3F, 0x0, '*', 0x0, 0x20};

    return scan_code_data[response];
}