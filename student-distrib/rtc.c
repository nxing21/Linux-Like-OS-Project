/* Virtualizes the RTC */

#include "rtc.h"
#include "lib.h"
#include "nmi.h"

#define RTC_IRQ  8

int RTC_frequency;

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
    outb (NMI_DISABLE_CMD | RTC_REG_A, RTC_REGISTER_SELECT);
    prev_data = inb(RTC_REGISTER_DATA_PORT);

    /* Reselects Register A. */
    outb(NMI_DISABLE_CMD | RTC_REG_A, RTC_REGISTER_SELECT);
    
    /* Sets RTC to the highest rate possible, 3. */
    outb((prev_data & 0xF0) | 0x03, RTC_REGISTER_DATA_PORT); /* 0xF0 saves the most significant 4 bits */

    /* Initializes the variable RTC_frequency to the current frequency */
    RTC_frequency = rtc_max_frequency >> (3-1);
   
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
void set_RTC_frequency(uint8_t rate){
    if (rate < 2 || rate > 15){ /* We can set the RTC rate from (2-15) */
        printf(("Rate is not valid!"));
    }
    else{
        RTC_frequency = rtc_max_frequency >> (rate - 1);
        printf(("RTC frequency set."));
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
    test_interrupts();

    /* Throws away the contents of Register C, allowing for interrupts to occur. */
    outb(RTC_REG_C, RTC_REGISTER_SELECT);
    garbage = inb(RTC_REGISTER_DATA_PORT);

    send_eoi(RTC_IRQ);

}
