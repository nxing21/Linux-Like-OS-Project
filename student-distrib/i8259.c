/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void i8259_init(void) {
    // Saves the masks
    master_mask = inb(MASTER_8259_PORT+1);
    slave_mask = inb(SLAVE_8259_PORT+1);

    // Starts the initializtion sequence
    outb(ICW1 , MASTER_8259_PORT);
    outb(ICW1, SLAVE_8259_PORT);
    outb(ICW2_MASTER, MASTER_8259_PORT+1); // Master PIC vector offset
    outb(ICW2_SLAVE, SLAVE_8259_PORT+1); // Slave PIC vector offset
    outb(ICW3_MASTER, MASTER_8259_PORT+1); // Tells Master PIC that there is a cascae
    outb(ICW3_SLAVE, SLAVE_8259_PORT+1); // Tells the Slave its cascade itentity (2)

    outb(ICW4, MASTER_8259_PORT+1); // Have the PICs use 8086 mode
    outb(ICW4, SLAVE_8259_PORT+1);

    outb(master_mask, MASTER_8259_PORT+1); // Restore the saved masks
    outb(slave_mask, SLAVE_8259_PORT+1); 
}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
    uint16_t port;
    uint8_t value;
    if (irq_num < 8){
        port = MASTER_8259_PORT+1;
    }
    else{
        port = SLAVE_8259_PORT+1;
        irq_num -= 8; 
    }

    value = inb(port) & ~(1 << irq_num);
    outb(value, port);

}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
    uint16_t port;
    uint8_t value;
    if (irq_num < 8){
        port = MASTER_8259_PORT+1;
    }
    else{
        port = SLAVE_8259_PORT+1;
        irq_num -= 8; 
    }

    value = inb(port) | (1 << irq_num);
    outb(value, port);
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
    if (irq_num >= 8){
        outb(EOI, SLAVE_8259_PORT);
    }
    outb(EOI, MASTER_8259_PORT);
}

/* Helper fucntion to get info from Interrupt Status Registers 
   The variable "ocw3" is the command word sent to the PICs that allows
   us to read the data from certain registers.
*/
uint16_t pic_get_int_reg(int ocw3){
    outb(ocw3, MASTER_8259_PORT);
    outb(ocw3, SLAVE_8259_PORT);

    return (inb(SLAVE_8259_PORT) << 8 | inb(MASTER_8259_PORT));
    /* Bits 0-7 are data from Master Port. Bits 8-15 are data from the Slave Port*/
    /* Note that bit 2 will always be 1 whenever bits 8-15 are set due to the
    cascaded nature of the PICs*/
}

/* Returns the combined value of the cascaded PICs irq request registers*/
uint16_t pic_get_irr(void){
    return pic_get_int_reg(PIC_READ_IRR);
}

/* Returns the combined value of the cascaded PICs in-serive registers*/
uint16_t pic_get_isr(void){
    return pic_get_int_reg(PIC_READ_ISR);
}
