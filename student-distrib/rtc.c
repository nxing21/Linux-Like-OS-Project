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

    /* Setting the RTC to the highest interrupt frequency.*/
    /* Select Register A and disables NMIs*/
    //outb (NMI_DISABLE_CMD | RTC_REG_A, RTC_REGISTER_SELECT);
    //prev_data = inb(RTC_REGISTER_DATA_PORT);

    /* Reselects Register A. */
    //outb(NMI_DISABLE_CMD | RTC_REG_A, RTC_REGISTER_SELECT);
    
    /* Sets RTC to the highest rate possible, 3. */
    //outb((prev_data & 0xF0) | 0x03, RTC_REGISTER_DATA_PORT);

    /* Initializes the variable RTC_frequency to the current frequency */
    // RTC_frequency = rtc_max_frequency >> (3-1);

    /* sets frequency to highest possible frequency */
    // set_RTC_frequency(rtc_lowest_rate);
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
 *   DESCRIPTION: Sets the RTC to a virtual frequency. This means that the RTC has to wait for x number of interrupts
 *                to equate to the set virtual frequency.
 *   INPUTS: rate - The rate we want to set the periodic interrupts to.
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Sets the RTC to a virtual frequency.
 */
void set_RTC_frequency(uint32_t rate) {
    if (rate >= rtc_lowest_rate && rate <= rtc_highest_rate) {
        rate &= 0x0F;
        cli();
        outb(NMI_DISABLE_CMD | RTC_REG_A, RTC_REGISTER_SELECT);
        char temp = inb(RTC_REGISTER_DATA_PORT);
        outb(NMI_DISABLE_CMD | RTC_REG_A, RTC_REGISTER_SELECT);
        outb((temp & 0xF0) | rate, RTC_REGISTER_DATA_PORT);
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
    uint8_t garbage;

    /* Function to test RTC */
    // test_interrupts();

    /* Throws away the contents of Register C, allowing for interrupts to occur. */
    outb(RTC_REG_C, RTC_REGISTER_SELECT);
    garbage = inb(RTC_REGISTER_DATA_PORT);

    if (RTC_counter == 0) {
        RTC_block = 0;
        RTC_counter = RTC_max_counter;
    } else {
        RTC_counter--;
    }

    send_eoi(RTC_IRQ);
}

// questions: is open supposed to do the job of init or does it just set freq to 2Hz?
// all functions should have parameters: open(filename) close(fd) read(fd)
// check parameter types
int RTC_open(const char* filename) {
    RTC_frequency = rtc_min_frequency;
    RTC_max_counter = rtc_max_usable_frequency / RTC_frequency;
    RTC_counter = RTC_max_counter;
    // set_RTC_frequency(rtc_highest_rate);
    return 0;
}

// questions: does it literally do nothing?
int RTC_close(uint32_t fd) {
    return 0;
}

// link: https://linux.die.net/man/2/read
int RTC_read(uint32_t fd, void* buffer, int nbytes) {
    RTC_block = 1;
    while (RTC_block == 1);
    return 0;
}

int RTC_write(uint32_t fd, void* buffer, int nbytes) {
    uint8_t i;
    unsigned int freq;

    if (nbytes != BYTE_4) {
        return -1;
    }

    if (buffer == NULL) {
        return -1;
    }

    freq = *((unsigned int*)(buffer));

    for (i = rtc_lowest_rate; i <= rtc_highest_rate; i++) {
        if (freq == (rtc_max_frequency >> (i-1))) {
            RTC_frequency = (rtc_max_frequency >> (i-1));
            RTC_max_counter = rtc_max_usable_frequency / RTC_frequency;
            RTC_counter = RTC_max_counter;
            // set_RTC_frequency(i);
            return 0;
        }
    }
    
    return -1;
}


