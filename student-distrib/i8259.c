/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* The default masks, all IRQs on the PIC are currently disabled. */
uint8_t master_mask = 0xFF; /* IRQs 0-7  */
uint8_t slave_mask = 0xFF;  /* IRQs 8-15 */

/* 
 * i9259_init
 *   DESCRIPTION: Sends preset command words to both the Master and Slave PIC for initialization.
 *                Enables IRQ2, as that allows the Slave PIC to send interrupts.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Initializes both the Master and Slave PICs.
 */
void i8259_init(void) {
    // Starts the initializtion sequence
    outb(ICW1, MASTER_8259_PORT);
    outb(ICW1, SLAVE_8259_PORT);

    outb(ICW2_MASTER, MASTER_8259_PORT+1); // Master PIC vector offset
    outb(ICW2_SLAVE, SLAVE_8259_PORT+1); // Slave PIC vector offset
    outb(ICW3_MASTER, MASTER_8259_PORT+1); // Tells Master PIC that there is a cascae
    outb(ICW3_SLAVE, SLAVE_8259_PORT+1); // Tells the Slave its cascade itentity (2)

    outb(ICW4, MASTER_8259_PORT+1); // Have the PICs use 8086 mode
    outb(ICW4, SLAVE_8259_PORT+1);
    outb(master_mask, MASTER_8259_PORT+1); // Set the mask.
    outb(slave_mask, SLAVE_8259_PORT+1); 

    enable_irq(SLAVE_PIC_IRQ); /* Enables the second PIC*/
}

/* 
 * enable_irq
 *   DESCRIPTION: Enables a certain IRQ on the PIC.
 *   INPUTS: irq_num - An IRQ number that corresponds to a certain device on the PIC.
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Allows for interrupts on a certain IRQ.
 */
void enable_irq(uint32_t irq_num) {
    uint16_t port;
    uint8_t value;
    if (irq_num > MAX_IRQS){ /* Not a valid IRQ */
        return;
    }
    if (irq_num < START_SLAVE_PIC){
        port = MASTER_8259_PORT+1;
    }
    else{
        port = SLAVE_8259_PORT+1;
        irq_num -= START_SLAVE_PIC; 
    }

    value = inb(port) & ~(1 << irq_num); /* Setting a certain bit of the PIC mask to 0, telling PIC to enable the corresponding IRQ*/
    outb(value, port);

}

/* 
 * disable_irq
 *   DESCRIPTION: Disbles a certain IRQ on the PIC.
 *   INPUTS: irq_num - An IRQ number that corresponds to a certain device on the PIC.
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Diasbles interrupts on a certain IRQ.
 */
void disable_irq(uint32_t irq_num) {
    uint16_t port;
    uint8_t value;
    if (irq_num > MAX_IRQS){ /* Not a valid IRQ */
        return;
    }
    if (irq_num < START_SLAVE_PIC){
        port = MASTER_8259_PORT+1;
    }
    else{
        port = SLAVE_8259_PORT+1;
        irq_num -= START_SLAVE_PIC; 
    }

    value = inb(port) | (1 << irq_num); /* Setting a certain bit of the PIC mask to 1, telling PIC to disable the corresponding IRQ*/
    outb(value, port);
}

/* 
 * send_eoi
 *   DESCRIPTION: Sends an EOI command OR'd with the IRQ number, to the PIC, indicating
 *                that a certain interrupt has been serviced.
 *   INPUTS: irq_num - An IRQ number that corresponds to a certain device on the PIC.
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Tells the PIC that a certain IRQ has been serviced.
 */
void send_eoi(uint32_t irq_num) {
    if (irq_num > MAX_IRQS){ /* Not a valid IRQ */
        return;
    }
    else if (irq_num < START_SLAVE_PIC){ /* IRQ came from the Master PIC */
        outb(EOI | irq_num, MASTER_8259_PORT);
    }
    else{ /* IRQ came from the Slave, must also unmask IRQ 2 on Master PIC */
        outb((EOI | irq_num) - START_SLAVE_PIC, SLAVE_8259_PORT);
        outb(EOI | SLAVE_PIC_IRQ, MASTER_8259_PORT);
    }
}
