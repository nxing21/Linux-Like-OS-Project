/* lib.c - Some basic library functions (printf, strlen, etc.)
 * vim:ts=4 noexpandtab */

#include "lib.h"
#include "terminal.h"
#include "keyboard.h"
#include "syscalls.h"

#define VIDEO       0xB8000
#define NUM_COLS    80
#define NUM_ROWS    25
#define CURSOR_LOC_HIGH_REG 0x0E
#define CURSOR_LOC_LOW_REG 0x0F
#define GET_8_MSB 8
#define GET_8_BITS 0xFF
#define CRTC_ADDR_PORT 0x3D4
#define CRTC_DATA_PORT 0x3D5

static char* video_mem = (char *)VIDEO;

int ATTRIB = 0x7;

// indicates which terminal to operate on, screen terminal (0) or current terminal (1) operated on by scheduler (may or may not be the same)
int terminal_flag = 0; 

/* void clear(void);
 * Inputs: void
 * Return Value: none
 * Function: Clears video memory on a specific terminal. */
void clear(void) {
    int32_t i;
    uint8_t ATTRIB;
    char *true_mem = video_mem;

    /* Checking if the terminal screen that we are editing is not currently on the screen. */
    if(curr_terminal != screen_terminal && (DISPLAY_ON_MAIN_PAGE == 0)) {
        true_mem = (char *) VIDEO_ADDR + ((curr_terminal+1) << 12);
        terminal_array[curr_terminal].screen_x=0;
        terminal_array[curr_terminal].screen_y=0;
        terminal_flag = 1;
        ATTRIB = terminal_array[curr_terminal].attribute;
    }
    /* Case where the screen that is being cleared is currently is on the screen. */
    else {
        DISPLAY_ON_MAIN_PAGE = 0; //setting flag back to zero, it's keyboard handlers jobs to let libc know each time
        terminal_array[screen_terminal].screen_x=0;
        terminal_array[screen_terminal].screen_y=0;
        ATTRIB = terminal_array[screen_terminal].attribute;
        terminal_flag = 0;
    }

    /* Set the whole screen to empty.*/
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        *(uint8_t *)(true_mem + (i << 1)) = ' ';
        *(uint8_t *)(true_mem + (i << 1) + 1) = ATTRIB;
    }
    move_cursor();
}

/* Standard printf().
 * Only supports the following format strings:
 * %%  - print a literal '%' character
 * %x  - print a number in hexadecimal
 * %u  - print a number as an unsigned integer
 * %d  - print a number as a signed integer
 * %c  - print a character
 * %s  - print a string
 * %#x - print a number in 32-bit aligned hexadecimal, i.e.
 *       print 8 hexadecimal digits, zero-padded on the left.
 *       For example, the hex number "E" would be printed as
 *       "0000000E".
 *       Note: This is slightly different than the libc specification
 *       for the "#" modifier (this implementation doesn't add a "0x" at
 *       the beginning), but I think it's more flexible this way.
 *       Also note: %x is the only conversion specifier that can use
 *       the "#" modifier to alter output. */
int32_t printf(int8_t *format, ...) {

    /* Pointer to the format string */
    int8_t* buf = format;

    /* Stack pointer for the other parameters */
    int32_t* esp = (void *)&format;
    esp++;

    while (*buf != '\0') {
        switch (*buf) {
            case '%':
                {
                    int32_t alternate = 0;
                    buf++;

format_char_switch:
                    /* Conversion specifiers */
                    switch (*buf) {
                        /* Print a literal '%' character */
                        case '%':
                            putc('%');
                            break;

                        /* Use alternate formatting */
                        case '#':
                            alternate = 1;
                            buf++;
                            /* Yes, I know gotos are bad.  This is the
                             * most elegant and general way to do this,
                             * IMHO. */
                            goto format_char_switch;

                        /* Print a number in hexadecimal form */
                        case 'x':
                            {
                                int8_t conv_buf[64];
                                if (alternate == 0) {
                                    itoa(*((uint32_t *)esp), conv_buf, 16);
                                    puts(conv_buf);
                                } else {
                                    int32_t starting_index;
                                    int32_t i;
                                    itoa(*((uint32_t *)esp), &conv_buf[8], 16);
                                    i = starting_index = strlen(&conv_buf[8]);
                                    while(i < 8) {
                                        conv_buf[i] = '0';
                                        i++;
                                    }
                                    puts(&conv_buf[starting_index]);
                                }
                                esp++;
                            }
                            break;

                        /* Print a number in unsigned int form */
                        case 'u':
                            {
                                int8_t conv_buf[36];
                                itoa(*((uint32_t *)esp), conv_buf, 10);
                                puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a number in signed int form */
                        case 'd':
                            {
                                int8_t conv_buf[36];
                                int32_t value = *((int32_t *)esp);
                                if(value < 0) {
                                    conv_buf[0] = '-';
                                    itoa(-value, &conv_buf[1], 10);
                                } else {
                                    itoa(value, conv_buf, 10);
                                }
                                puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a single character */
                        case 'c':
                            putc((uint8_t) *((int32_t *)esp));
                            esp++;
                            break;

                        /* Print a NULL-terminated string */
                        case 's':
                            puts(*((int8_t **)esp));
                            esp++;
                            break;

                        default:
                            break;
                    }

                }
                break;

            default:
                putc(*buf);
                break;
        }
        buf++;
    }
    return (buf - format);
}

/* int32_t puts(int8_t* s);
 *   Inputs: int_8* s = pointer to a string of characters
 *   Return Value: Number of bytes written
 *    Function: Output a string to the console */
int32_t puts(int8_t* s) {
    register int32_t index = 0;
    while (s[index] != '\0') {
        putc(s[index]);
        index++;
    }
    return index;
}

/* void putc(uint8_t c);
 * Inputs: uint_8* c = character to print
 * Return Value: void
 *  Function: Output a character to the respective console */
void putc(uint8_t c) {
    int i, j;
    uint8_t ATTRIB;
    uint8_t character;
    char *true_mem = video_mem;
    int true_term_id = screen_terminal;

    /* Check if the terminal that we want to edit is currently on screen. */
    if(curr_terminal != screen_terminal && (DISPLAY_ON_MAIN_PAGE == 0)) {
        true_mem = (char *) VIDEO_ADDR + ((curr_terminal+1) << 12);
        true_term_id = curr_terminal;
        terminal_flag = 1;
        ATTRIB = terminal_array[curr_terminal].attribute;
    }
    /* Case where the terminal we are trying to edit is on screen. */
    else {
        DISPLAY_ON_MAIN_PAGE = 0; //setting flag back to zero, it's keyboard handlers jobs to let libc know each time
        terminal_flag = 0;
        ATTRIB = terminal_array[screen_terminal].attribute;
    }

    if(c == '\n' || c == '\r') {
        terminal_array[true_term_id].screen_y++;
        terminal_array[true_term_id].screen_x = 0;
    } 
    else {
        *(uint8_t *)(true_mem + ((NUM_COLS * terminal_array[true_term_id].screen_y + terminal_array[true_term_id].screen_x) << 1)) = c;
        *(uint8_t *)(true_mem + ((NUM_COLS * terminal_array[true_term_id].screen_y + terminal_array[true_term_id].screen_x) << 1) + 1) = ATTRIB;
        terminal_array[true_term_id].screen_x++;
        check_size(); /* Checks if we went out of bounds. */
    }

    /* Checks if the y position exceeds the window. If so, scroll up and reposition the cursor.*/
    if (terminal_array[true_term_id].screen_y > NUM_ROWS-1){
        for (i = 0; i < NUM_ROWS-1; i++){
            for (j = 0; j < NUM_COLS; j++){
                /* For each iteration, replace the top row with the row below it. */
                character = *(uint8_t *)(true_mem + ((NUM_COLS * (i+1) + j) << 1));
                *(uint8_t *)(true_mem + ((NUM_COLS * i + j) << 1)) = character;
                *(uint8_t *)(true_mem + ((NUM_COLS * i + j) << 1) + 1) = ATTRIB;
            }
        }

        /* Separate case for printing the last row, since there is no row below it. */
        i = NUM_ROWS-1;
        for (j = 0; j < NUM_COLS; j++){
            *(uint8_t *)(true_mem + ((NUM_COLS * (i) + j) << 1)) = 0x0;
            *(uint8_t *)(true_mem + ((NUM_COLS * (i) + j) << 1) + 1) = ATTRIB;
        }
        terminal_array[true_term_id].screen_y = NUM_ROWS-1;
        terminal_array[true_term_id].screen_x = 0;
    }
    move_cursor();
}

/* int8_t* itoa(uint32_t value, int8_t* buf, int32_t radix);
 * Inputs: uint32_t value = number to convert
 *            int8_t* buf = allocated buffer to place string in
 *          int32_t radix = base system. hex, oct, dec, etc.
 * Return Value: number of bytes written
 * Function: Convert a number to its ASCII representation, with base "radix" */
int8_t* itoa(uint32_t value, int8_t* buf, int32_t radix) {
    static int8_t lookup[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int8_t *newbuf = buf;
    int32_t i;
    uint32_t newval = value;

    /* Special case for zero */
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return buf;
    }

    /* Go through the number one place value at a time, and add the
     * correct digit to "newbuf".  We actually add characters to the
     * ASCII string from lowest place value to highest, which is the
     * opposite of how the number should be printed.  We'll reverse the
     * characters later. */
    while (newval > 0) {
        i = newval % radix;
        *newbuf = lookup[i];
        newbuf++;
        newval /= radix;
    }

    /* Add a terminating NULL */
    *newbuf = '\0';

    /* Reverse the string and return */
    return strrev(buf);
}

/* int8_t* strrev(int8_t* s);
 * Inputs: int8_t* s = string to reverse
 * Return Value: reversed string
 * Function: reverses a string s */
int8_t* strrev(int8_t* s) {
    register int8_t tmp;
    register int32_t beg = 0;
    register int32_t end = strlen(s) - 1;

    while (beg < end) {
        tmp = s[end];
        s[end] = s[beg];
        s[beg] = tmp;
        beg++;
        end--;
    }
    return s;
}

/* uint32_t strlen(const int8_t* s);
 * Inputs: const int8_t* s = string to take length of
 * Return Value: length of string s
 * Function: return length of string s */
uint32_t strlen(const int8_t* s) {
    register uint32_t len = 0;
    while (s[len] != '\0')
        len++;
    return len;
}

/* void* memset(void* s, int32_t c, uint32_t n);
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set n consecutive bytes of pointer s to value c */
void* memset(void* s, int32_t c, uint32_t n) {
    c &= 0xFF;
    asm volatile ("                 \n\
            .memset_top:            \n\
            testl   %%ecx, %%ecx    \n\
            jz      .memset_done    \n\
            testl   $0x3, %%edi     \n\
            jz      .memset_aligned \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            subl    $1, %%ecx       \n\
            jmp     .memset_top     \n\
            .memset_aligned:        \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            movl    %%ecx, %%edx    \n\
            shrl    $2, %%ecx       \n\
            andl    $0x3, %%edx     \n\
            cld                     \n\
            rep     stosl           \n\
            .memset_bottom:         \n\
            testl   %%edx, %%edx    \n\
            jz      .memset_done    \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            subl    $1, %%edx       \n\
            jmp     .memset_bottom  \n\
            .memset_done:           \n\
            "
            :
            : "a"(c << 24 | c << 16 | c << 8 | c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memset_word(void* s, int32_t c, uint32_t n);
 * Description: Optimized memset_word
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set lower 16 bits of n consecutive memory locations of pointer s to value c */
void* memset_word(void* s, int32_t c, uint32_t n) {
    asm volatile ("                 \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            cld                     \n\
            rep     stosw           \n\
            "
            :
            : "a"(c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memset_dword(void* s, int32_t c, uint32_t n);
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set n consecutive memory locations of pointer s to value c */
void* memset_dword(void* s, int32_t c, uint32_t n) {
    asm volatile ("                 \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            cld                     \n\
            rep     stosl           \n\
            "
            :
            : "a"(c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memcpy(void* dest, const void* src, uint32_t n);
 * Inputs:      void* dest = destination of copy
 *         const void* src = source of copy
 *              uint32_t n = number of byets to copy
 * Return Value: pointer to dest
 * Function: copy n bytes of src to dest */
void* memcpy(void* dest, const void* src, uint32_t n) {
    asm volatile ("                 \n\
            .memcpy_top:            \n\
            testl   %%ecx, %%ecx    \n\
            jz      .memcpy_done    \n\
            testl   $0x3, %%edi     \n\
            jz      .memcpy_aligned \n\
            movb    (%%esi), %%al   \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            addl    $1, %%esi       \n\
            subl    $1, %%ecx       \n\
            jmp     .memcpy_top     \n\
            .memcpy_aligned:        \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            movl    %%ecx, %%edx    \n\
            shrl    $2, %%ecx       \n\
            andl    $0x3, %%edx     \n\
            cld                     \n\
            rep     movsl           \n\
            .memcpy_bottom:         \n\
            testl   %%edx, %%edx    \n\
            jz      .memcpy_done    \n\
            movb    (%%esi), %%al   \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            addl    $1, %%esi       \n\
            subl    $1, %%edx       \n\
            jmp     .memcpy_bottom  \n\
            .memcpy_done:           \n\
            "
            :
            : "S"(src), "D"(dest), "c"(n)
            : "eax", "edx", "memory", "cc"
    );
    return dest;
}

/* void* memmove(void* dest, const void* src, uint32_t n);
 * Description: Optimized memmove (used for overlapping memory areas)
 * Inputs:      void* dest = destination of move
 *         const void* src = source of move
 *              uint32_t n = number of byets to move
 * Return Value: pointer to dest
 * Function: move n bytes of src to dest */
void* memmove(void* dest, const void* src, uint32_t n) {
    asm volatile ("                             \n\
            movw    %%ds, %%dx                  \n\
            movw    %%dx, %%es                  \n\
            cld                                 \n\
            cmp     %%edi, %%esi                \n\
            jae     .memmove_go                 \n\
            leal    -1(%%esi, %%ecx), %%esi     \n\
            leal    -1(%%edi, %%ecx), %%edi     \n\
            std                                 \n\
            .memmove_go:                        \n\
            rep     movsb                       \n\
            "
            :
            : "D"(dest), "S"(src), "c"(n)
            : "edx", "memory", "cc"
    );
    return dest;
}

/* int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n)
 * Inputs: const int8_t* s1 = first string to compare
 *         const int8_t* s2 = second string to compare
 *               uint32_t n = number of bytes to compare
 * Return Value: A zero value indicates that the characters compared
 *               in both strings form the same string.
 *               A value greater than zero indicates that the first
 *               character that does not match has a greater value
 *               in str1 than in str2; And a value less than zero
 *               indicates the opposite.
 * Function: compares string 1 and string 2 for equality */
int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n) {
    int32_t i;
    for (i = 0; i < n; i++) {
        if ((s1[i] != s2[i]) || (s1[i] == '\0') /* || s2[i] == '\0' */) {

            /* The s2[i] == '\0' is unnecessary because of the short-circuit
             * semantics of 'if' expressions in C.  If the first expression
             * (s1[i] != s2[i]) evaluates to false, that is, if s1[i] ==
             * s2[i], then we only need to test either s1[i] or s2[i] for
             * '\0', since we know they are equal. */
            return s1[i] - s2[i];
        }
    }
    return 0;
}

/* int8_t* strcpy(int8_t* dest, const int8_t* src)
 * Inputs:      int8_t* dest = destination string of copy
 *         const int8_t* src = source string of copy
 * Return Value: pointer to dest
 * Function: copy the source string into the destination string */
int8_t* strcpy(int8_t* dest, const int8_t* src) {
    int32_t i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return dest;
}

/* int8_t* strcpy(int8_t* dest, const int8_t* src, uint32_t n)
 * Inputs:      int8_t* dest = destination string of copy
 *         const int8_t* src = source string of copy
 *                uint32_t n = number of bytes to copy
 * Return Value: pointer to dest
 * Function: copy n bytes of the source string into the destination string */
int8_t* strncpy(int8_t* dest, const int8_t* src, uint32_t n) {
    int32_t i = 0;
    while (src[i] != '\0' && i < n) {
        dest[i] = src[i];
        i++;
    }
    while (i < n) {
        dest[i] = '\0';
        i++;
    }
    return dest;
}

/* void test_interrupts(void)
 * Inputs: void
 * Return Value: void
 * Function: increments video memory. To be used to test rtc */
void test_interrupts(void) {
    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        video_mem[i << 1]++;
    }
}

/*  check_size()
 * Inputs: None
 * Return Value: None
 * Function: Checks the position of x and y to see if its out of bounds. */
void check_size(){
    int true_terminal = screen_terminal;

    /* Checks what terminal we are trying to edit. */
    if(terminal_flag == 1) {
        true_terminal = curr_terminal;
    }

    /* If we go out of bounds in the x direction, make screen_x = 0 and move screen_y down one row. */
    if (terminal_array[true_terminal].screen_x >= NUM_COLS){ 
        terminal_array[true_terminal].screen_y++;
        terminal_array[true_terminal].screen_x = 0;
    }
}

/*  erase_char()
 * Inputs: None
 * Return Value: None
 * Function: Deletes a character from the screen of the terminal that we are looking at. */
void erase_char(){
    /*erase_char is only used by typing triggerd cursor movement, meaning only screen terminal*/
    terminal_flag = 0;
    /* Deletes the previous character. */
    terminal_array[screen_terminal].screen_x--;

    /* Checks if we went out of bounds in the direction. If so, place screen_x at the very last column and move screen_y up one row. */
    if (terminal_array[screen_terminal].screen_x < 0){
        terminal_array[screen_terminal].screen_x = NUM_COLS-1;
        terminal_array[screen_terminal].screen_y--;
    }
    *(uint8_t *)(video_mem + ((NUM_COLS * terminal_array[screen_terminal].screen_y + terminal_array[screen_terminal].screen_x) << 1)) = 0x0;

    /* Moves the cursor*/
    move_cursor();
}

/*  move_cursor()
 * Inputs: None
 * Return Value: None
 * Function: Moves the cursor based on the position of x and y*/
void move_cursor(){
    uint16_t position;

    /* Use the screen_x and screen_y of the terminal that we are currently looking at. */
    position = terminal_array[screen_terminal].screen_y* NUM_COLS + terminal_array[screen_terminal].screen_x;

    /* Edit ports to move the cursor. */
    outb(CURSOR_LOC_LOW_REG, CRTC_ADDR_PORT);
    outb((uint8_t)(position & GET_8_BITS), CRTC_DATA_PORT);
    outb(CURSOR_LOC_HIGH_REG,CRTC_ADDR_PORT);
    outb((uint8_t)((position >> GET_8_MSB) & GET_8_BITS), CRTC_DATA_PORT);
}
