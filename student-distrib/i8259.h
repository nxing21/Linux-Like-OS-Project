/* i8259.h - Defines used in interactions with the 8259 interrupt
 * controller
 * vim:ts=4 noexpandtab
 */

#ifndef _I8259_H
#define _I8259_H

#include "types.h"

/* Ports that each PIC sits on */
#define MASTER_8259_PORT    0x20
#define SLAVE_8259_PORT     0xA0

/* Initialization control words to init each PIC.
 * See the Intel manuals for details on the meaning
 * of each word */
#define ICW1                0x11 // Initialization keyword
#define ICW2_MASTER         0x20 // ICW2: Stores information regarding
#define ICW2_SLAVE          0x28 // the interrupt vector address
#define ICW3_MASTER         0x04 // ICW3: Used when there is more than one
#define ICW3_SLAVE          0x02 // PIC present, loads an 8-bit slave register
#define ICW4                0x01 // 8086 operations are performed

#define PIC_READ_IRR        0x0A // Command word that allows us to read IRR
#define PIC_READ_ISR        0x0B // Command word that allows us to read ISR

/* End-of-interrupt byte.  This gets OR'd with
 * the interrupt number and sent out to the PIC
 * to declare the interrupt finished */
#define EOI                 0x60

/* Externally-visible functions */

/* Initialize both PICs */
void i8259_init(void);
/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num);
/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num);
/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num);
/* Helper function to get information from the Interrupt Registers*/
uint16_t pic_get_int_reg(int ocw3);
/* Returns the combined value of the cascaded PICs irq request registers*/
uint16_t pic_get_irr(void);
/* Returns the combined value of the cascaded PICs in-serive registers*/
uint16_t pic_get_isr(void);
#endif /* _I8259_H */
