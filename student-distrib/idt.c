/* idt.c - Everything to do with the idt
 */

#include "idt.h"
#include "x86_desc.h"
#include "lib.h"



/* Initialize the idt */
void idt_init() {
    // lidt macro
    lidt(ldt_desc_ptr);
    // build idt
    build_idt();
}

void build_idt() {
    int i;

    for (i = 0; i < IDT_LENGTH; i++) {
        // idt[i].offset_15_00 = ?
        idt[i].seg_selector = KERNEL_CS;
        idt[i].reserved4 = 0;
        idt[i].reserved3 = 0;
        idt[i].reserved2 = 1;
        idt[i].reserved1 = 1;
        // idt[i].size = size of gate: 1 = 32bits, 0 = 16bits
        idt[i].reserved0 = 0;
        // idt[i].dpl = descriptor privilege level
        // idt[i].present = segment present flag
        // idt[i].offset_31_16 = ?
    }
}

