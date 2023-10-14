/* Virtualizes the RTC */
#include "lib.h"
#include "nmi.h"
#include "i8259.h"
#include "rtc.h"

#define RTC_IRQ  8

uint8_t RTC_frequency;

/* Initalizes the RTC. */
void init_RTC(){
    uint8_t prev_data;

    /*Selects Register B and disables NMIs */
    outb(NMI_DISABLE_CMD | RTC_REG_B, RTC_REGISTER_SELECT);

    /* Gets the data from Register B*/
    prev_data = inb(RTC_REGISTER_DATA_PORT);

    /* Select Register B again */
    outb(NMI_DISABLE_CMD | RTC_REG_B, RTC_REGISTER_SELECT);

    /* Turns on periodic interrupts by setting bit 6 of prev data to 1 */
    outb(prev_data | 0x40, RTC_REGISTER_DATA_PORT);

    /* Setting the RTC to the highest interrupt frequency.*/
    /* Select Register A and disables NMIs*/
    outb (NMI_DISABLE_CMD | RTC_REG_A, RTC_REGISTER_SELECT);
    prev_data = inb(RTC_REGISTER_DATA_PORT);

    /* Reselects Register A. */
    outb(NMI_DISABLE_CMD | RTC_REG_A, RTC_REGISTER_SELECT);
    
    /* Sets RTC to the highest rate possible, 3. */
    outb((prev_data & 0xF0) | 0x03, RTC_REGISTER_DATA_PORT);

    /* Initializes the variable RTC_frequency to the current frequency */
    RTC_frequency = rtc_max_frequency >> (3-1);

    /* Enables IRQ 8 on the PIC */
    enable_irq(RTC_IRQ);

    /* Renables NMIs */
    NMI_enable();
} 

/* Creates a VIRTUAL frequency, waiting until a certain number of interrupts happen to generate a */
void set_RTC_frequency(uint8_t rate){
    if (rate < 2 || rate > 15){
        printf(("Rate is not valid!"));
    }
    else{
        RTC_frequency = rtc_max_frequency >> (rate - 1);
        printf(("RTC frequency set."));
    }
}

/* Handles RTC interrupts */
void RTC_handler(){

    /* Throws away the contents of Register C, allowing for interrupts to occur. */
    outb(RTC_REG_C, RTC_REGISTER_SELECT);
    inb(RTC_REGISTER_DATA_PORT);

}