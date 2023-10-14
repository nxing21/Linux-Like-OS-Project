#ifndef IDT_H
#define IDT_H

#define IDT_LENGTH  0x14    /* length of idt */

/* Initialize the idt */
void idt_init();

/* Build idt */
void build_idt();

#endif /* IDT_H */

