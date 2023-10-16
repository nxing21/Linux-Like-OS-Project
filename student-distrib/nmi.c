#include "nmi.h"
#include "lib.h"
/* IMPORTANT: Be careful about disabling NMIs for extended periods of time. */

/* 
 * NMI_enable
 *   DESCRIPTION: Enables non-maskable interrupts by sending a command word to port 0x70.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Enables non-maskable interrupts.
 */
void NMI_enable(){
    uint8_t NMI_data;

    /* Sets bit 7 to 0, renabling NMIs */
    NMI_data = inb(NMI_PORT) & NMI_ENABLE_CMD;
    outb(NMI_data, NMI_PORT);

    /* Clears the input buffer */
    NMI_data = inb(NMI_PORT);
    
}

/* 
 * NMI_disable
 *   DESCRIPTION: Disables non-maskalbe interrupts by sending a command word to port 0x70.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Disables non-maskable interrupts.
 */
void NMI_disable(){
    uint8_t NMI_data;

    /* Sets bit 7 to 1, disabling NMIs*/
    NMI_data = inb(NMI_PORT) | NMI_DISABLE_CMD;
    outb(NMI_data, NMI_PORT);

    /* Clears the input buffer. */
    NMI_data = inb(NMI_PORT);
}
