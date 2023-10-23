/* Virtualizes the RTC */

#include "rtc.h"
#include "lib.h"
#include "nmi.h"

#define RTC_IRQ         8
#define BYTE_4          4
#define RATE_OFFSET     3

int RTC_frequency, RTC_max_counter;
volatile int RTC_block, RTC_counter;

/* 
 * init_RTC
 *   DESCRIPTION: Initializes the RTC by enabling IRQ8 on the PIC, turning on periodic interrupts on the RTC,
 *                and setting the RTC to the highest frequency possible.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Allows for the RTC to send periodic interrupts.
 */
void init_RTC(){
    uint8_t prev_data;
    /*Selects Register B and disables NMIs */
    outb(NMI_DISABLE_CMD | RTC_REG_B, RTC_REGISTER_SELECT);

    /* Gets the data from Register B*/
    prev_data = inb(RTC_REGISTER_DATA_PORT);

    /* Select Register B again */
    outb(NMI_DISABLE_CMD | RTC_REG_B, RTC_REGISTER_SELECT);

    /* Turns on periodic interrupts by setting bit 6 of prev data to 1 (hence 0x40) */
    outb(prev_data | 0x40, RTC_REGISTER_DATA_PORT);

    /* sets frequency to highest possible frequency */
    RTC_frequency = rtc_max_usable_frequency;
    RTC_max_counter = rtc_max_usable_frequency / RTC_frequency;
    RTC_counter = RTC_max_counter;

    /* initializes RTC_block */
    RTC_block = 0;
   
    /* Renables NMIs */
    NMI_enable();
    
    /* Enables IRQ 8 on the PIC */
    enable_irq(RTC_IRQ);
} 

/* 
 * set_RTC_frequency
 *   DESCRIPTION: Sets the RTC to a specific frequency. (NOT USED WITH VIRTUALIZTAION)
 *   INPUTS: rate - The rate we want to set the periodic interrupts to.
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Enables NMIs. Ends critical section.
 */
void set_RTC_frequency(uint32_t rate) {
    if (rate >= rtc_lowest_rate && rate <= rtc_highest_rate) {
        rate &= 0x0F;   // bit mask: 0x0F

        cli();

        /* Select Register A and disables NMIs*/
        outb(NMI_DISABLE_CMD | RTC_REG_A, RTC_REGISTER_SELECT);
        /* Gets the data from Register B*/
        char temp = inb(RTC_REGISTER_DATA_PORT);
        /* Reselects Register A. */
        outb(NMI_DISABLE_CMD | RTC_REG_A, RTC_REGISTER_SELECT);
        /* Sets RTC to the given rate */
        outb((temp & 0xF0) | rate, RTC_REGISTER_DATA_PORT);
        /* Enables NMIs */
        NMI_enable();

        sti();
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
void RTC_handler(){
    uint8_t garbage;   // garbage

    /* Function to test RTC */
    // test_interrupts();

    /* Throws away the contents of Register C, allowing for interrupts to occur. */
    outb(RTC_REG_C, RTC_REGISTER_SELECT);
    garbage = inb(RTC_REGISTER_DATA_PORT);

    // Sets RTC_counter for RTC_read
    if (RTC_counter == 0) {
        RTC_block = 0;
        RTC_counter = RTC_max_counter;
    } else {
        RTC_counter--;
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
int RTC_open(const uint8_t* filename) {
    // sets RTC frequency to 2Hz
    RTC_frequency = rtc_min_frequency;
    RTC_max_counter = rtc_max_usable_frequency / RTC_frequency;
    RTC_counter = RTC_max_counter;
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
int RTC_close(int32_t fd) {
    return 0;
}

/* 
 * RTC_read
 *   DESCRIPTION: Blocks until the next interrupt (at correct frequency).
 *   INPUTS: fd -- file descriptor
             buffer -- void* to buffer
             nbytes -- num bytes
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none
 */
int RTC_read(int32_t fd, void* buffer, int32_t nbytes) {
    RTC_block = 1;
    while (RTC_block == 1);
    return 0;
}

/* 
 * RTC_write
 *   DESCRIPTION: Sets RTC frequency to specified frequency.
 *   INPUTS: fd -- file descriptor
             buffer -- void* to freq
             nbytes -- num bytes
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success; -1 on failure
 *   SIDE EFFECTS: none
 */
int RTC_write(int32_t fd, const void* buffer, int32_t nbytes) {
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
            RTC_frequency = (rtc_max_frequency >> (i-1));   // rate to frequency formula: freq = max_freq >> (rate-1)
            RTC_max_counter = rtc_max_usable_frequency / RTC_frequency;
            RTC_counter = RTC_max_counter;
            return 0;
        }
    }
    
    return -1;
}


