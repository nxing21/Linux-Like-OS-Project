/* Virtualizes the RTC */

#include "rtc.h"
#include "lib.h"
#include "syscalls.h"
#include "terminal.h"

#define RTC_IRQ         8
#define BYTE_4          4
#define RATE_OFFSET     3

int RTC_frequency;
volatile int RTC_max_counter[MAX_TERMINALS]; // each terminal has it's own max
volatile int RTC_block[MAX_TERMINALS]; //each terminal gets a separate RTC block
volatile int RTC_counter[MAX_TERMINALS]; // each terminal gets a separate RTC counter

/* 
 * init_RTC
 *   DESCRIPTION: Initializes the RTC by enabling IRQ8 on the PIC, turning on periodic interrupts on the RTC,
 *                and setting the RTC to the highest frequency possible.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Allows for the RTC to send periodic interrupts.
 */
void init_RTC() {
    uint8_t prev_data;

    /* Source: https://wiki.osdev.org/RTC */

    int i;
    /*Selects Register B and disables NMIs */
    outb(NMI_DISABLE_CMD | RTC_REG_B, RTC_REGISTER_SELECT);

    /* Gets the data from Register B*/
    prev_data = inb(RTC_REGISTER_DATA_PORT);

    /* Select Register B again */
    outb(NMI_DISABLE_CMD | RTC_REG_B, RTC_REGISTER_SELECT);

    /* Turns on periodic interrupts by setting bit 6 of prev data to 1 (hence 0x40) */
    outb(prev_data | 0x40, RTC_REGISTER_DATA_PORT);
    
    /* Selects Register A. */
    outb(NMI_DISABLE_CMD | RTC_REG_A, RTC_REGISTER_SELECT);

    /* Gets the data from Register A*/
    prev_data = inb(RTC_REGISTER_DATA_PORT);

    /* Selects Register A again. */
    outb(NMI_DISABLE_CMD | RTC_REG_A, RTC_REGISTER_SELECT);

    /* Setting the lowest frequency, by setting the rate equal to 3. 0xF0 bit masks our previous data, allowing us to edit the lower 4 bits. */
    outb((prev_data & 0xF0)| RATE_OF_LOW_FREQ, RTC_REGISTER_DATA_PORT);


    /* initializes RTC variables */
    for(i = 0; i < MAX_TERMINALS; i++){
            RTC_block[i] = 0;
            RTC_max_counter[i] = 1;
            RTC_counter[i] = 1;
    }
   
    /* Renables NMIs */
    /* Sets bit 7 to 0, renabling NMIs */
    prev_data = inb(NMI_PORT) & NMI_ENABLE_CMD;
    outb(prev_data, NMI_PORT);
    
    /* Clears the input buffer */
    prev_data = inb(NMI_PORT);
    
    /* Enables IRQ 8 on the PIC */
    enable_irq(RTC_IRQ);
} 

/* 
 * set_RTC_frequency
 *   DESCRIPTION: Sets the RTC to the specified virtual frequency.
 *   INPUTS: freq - The frequency we want to set the periodic interrupts to.
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void set_RTC_frequency(int freq) {
    if (freq >= rtc_min_frequency && freq <= rtc_max_usable_frequency) {
        RTC_frequency = freq;
        RTC_max_counter[curr_terminal] = rtc_max_usable_frequency / RTC_frequency;
        RTC_counter[curr_terminal] = RTC_max_counter[curr_terminal];
    }
}

/* 
 * RTC_handler
 *   DESCRIPTION: Handles interrupts from the RTC. After servicing, an EOI is sent to the PIC.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Handles the interrupt from the RTC, and sends and EOI to the PIC.
 */
void RTC_handler() {
    uint8_t garbage;   // garbage
    int i;

    /* Throws away the contents of Register C, allowing for interrupts to occur. */
    outb(RTC_REG_C, RTC_REGISTER_SELECT);
    garbage = inb(RTC_REGISTER_DATA_PORT);

    for(i = 0; i < MAX_TERMINALS; i++){
        // Sets RTC_counter for RTC_read
        if (RTC_counter[i] == 0) { // once counter = 0 change block to 0 indicating a tick
            RTC_block[i] = 0;
            RTC_counter[i] = RTC_max_counter[i];
        } else {
            RTC_counter[i]--; 
        }
    }
    

    send_eoi(RTC_IRQ);
}

/* 
 * RTC_open
 *   DESCRIPTION: Sets RTC frequency to 2Hz.
 *   INPUTS: filename -- filename
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none
 */
int32_t RTC_open(const uint8_t* filename) {
    // sets RTC frequency to 2Hz
    set_RTC_frequency(rtc_min_frequency);
    return 0;
}

/* 
 * RTC_close
 *   DESCRIPTION: Returns 0.
 *   INPUTS: fd -- file descriptor
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none
 */
int32_t RTC_close(int32_t fd) {
    return 0;
}

/* 
 * RTC_read
 *   DESCRIPTION: Blocks until the next interrupt (at correct frequency).
 *   INPUTS: fd -- file descriptor
             buffer -- pointer to buffer
             nbytes -- number of bytes
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none
 */
int32_t RTC_read(int32_t fd, void* buffer, int32_t nbytes) {

    RTC_counter[curr_terminal] = RTC_max_counter[curr_terminal]; //set counter to max (start value)
    RTC_block[curr_terminal] = 1; // initialize block to 1

    while (1){
        if (RTC_block[curr_terminal] == 0){ //once counter hits 0 block becomes 0 and we can return from read
            break;
        }
    }
    return 0;
}

/* 
 * RTC_write
 *   DESCRIPTION: Sets RTC frequency to specified frequency.
 *   INPUTS: fd -- file descriptor
             buffer -- pointer to freq
             nbytes -- number of bytes
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success; -1 on failure
 *   SIDE EFFECTS: none
 */
int32_t RTC_write(int32_t fd, const void* buffer, int32_t nbytes) {
    uint8_t i;   // looping variable
    unsigned int freq;   // frequency temp variable

    if (nbytes != BYTE_4) {
        return -1;
    }

    if (buffer == NULL) {
        return -1;
    }
    
    // obtains frequency from buffer
    freq = *((unsigned int*)(buffer));

    // loops through all possible rates
    for (i = rtc_lowest_rate; i <= rtc_highest_rate; i++) {
        if (freq == (rtc_max_frequency >> (i-1))) {   // rate to frequency formula: freq = max_freq >> (rate-1)
            // sets frequency to specified frequency
            set_RTC_frequency(rtc_max_frequency >> (i-1));   // rate to frequency formula: freq = max_freq >> (rate-1)
            return 0;
        }
    }
    
    return -1;
}


