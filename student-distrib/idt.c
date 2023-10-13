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
        idt[i].size = 1; // size of gate: 1 = 32bits, 0 = 16bits
        idt[i].reserved0 = 0;
        idt[i].dpl = 0; // kernel level, descriptor privilege level
        idt[i].present = 1; // segment present flag
        // idt[i].offset_31_16 = ?

        // for offset, we need to call SET_IDT_ENTRY
        SET_IDT_ENTRY(idt[i], errors[i]);
    }
}

void exception_handler(int exception_num) {
    // do stuff
}

static inline void* errors = {divide_error, debug, nmi, breakpoint, overflow, bound_range_exceeded, invalid_opcode, device_not_available, double_fault, coprocessor_segment_overrun, 
invalid_tss, segment_not_present, stack_fault, general_protection, page_fault, NULL, x87_fp_error, alignment_check, machin_check, simd_fp_error};
// idt[0], divide_error
// idt[1], debug
// idt[2], nmi
// idt[3], breakpoint
// idt[4], overflow
// idt[5], bound_range_exceeded
// idt[6], invalid_opcode
// idt[7], device_not_available
// idt[8], double_fault
// idt[9], coprocessor_segment_overrun
// idt[10], invalid_tss
// idt[11], segment_not_present
// idt[12], stack_fault
// idt[13], general_protection
// idt[14], page_fault
// idt[16], x87_fp_error
// idt[17], alignment_check
// idt[18], machine_check
// idt[19], simd_fp_error
