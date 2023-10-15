/* Deals with non-maskable interrupts */
/* We could probably move this and nmi.c to a different file, if necessary. */

#define NMI_PORT    0x70
#define NMI_ENABLE_CMD 0x7F /* Sets bit 7 in the NMI port to 0, enabling NMIs. Use bitwise AND. */
#define NMI_DISABLE_CMD  0x80 /* Sets bit 7 in the NMI port to 1, disabling NMIs. Use bitwise OR. */
 

void NMI_enable();
void NMI_disable();