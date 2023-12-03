#include "idt.h"
#include "syscalls_linkage.h"
#include "syscalls.h"

/* idt_init()
 * Inputs: none
 * Return Value: none
 * Function: Initializes the IDT and loads the IDTR register
 */
void idt_init() {
    /* Initializes the IDT */
    build_idt();
    /* Loads the IDTR register */
    lidt(idt_desc_ptr);
}

/* build_idt()
 * Inputs: none
 * Return Value: none
 * Function: Builds the IDT
 */
void build_idt() {
    int i; // loop counter

    for (i = 0; i < NUM_VEC; i++) { // iterate though IDT
        /*
         * Set everything according to trap gate.
         * See ISA manual page 156.
         */
        idt[i].seg_selector = KERNEL_CS;
        idt[i].reserved4 = 0;
        idt[i].reserved3 = 1;
        idt[i].reserved2 = 1;
        idt[i].reserved1 = 1;
        idt[i].size = 1;
        idt[i].reserved0 = 0;
        idt[i].dpl = 0;
        idt[i].present = 0;
        if (i <= NUM_EXCEPTIONS && i != SKIP) {
            idt[i].present = 1;
        }
        if (i == SYSTEM_CALL_VECTOR) {
            idt[i].present = 1;
            idt[i].dpl = 3; // set privilege level for syscalls to 3 (11 in binary)
        }
    }

    /* 
     * Set first 20 entries of IDT with their
     * corresponding exception function pointer.
     */
    SET_IDT_ENTRY(idt[DIVIDE_ERROR], divide_error);
    SET_IDT_ENTRY(idt[DEBUG], debug);
    SET_IDT_ENTRY(idt[NMI], nmi);
    SET_IDT_ENTRY(idt[BREAKPOINT], breakpoint);
    SET_IDT_ENTRY(idt[OVERFLOW], overflow);
    SET_IDT_ENTRY(idt[BOUND_RANGE_EXCEEDED], bound_range_exceeded);
    SET_IDT_ENTRY(idt[INVALID_OPCODE], invalid_opcode);
    SET_IDT_ENTRY(idt[DEVICE_NOT_AVAILABLE], device_not_available);
    SET_IDT_ENTRY(idt[DOUBLE_FAULT], double_fault);
    SET_IDT_ENTRY(idt[COPROCESSOR_SEGMENT_OVERRUN], coprocessor_segment_overrun);
    SET_IDT_ENTRY(idt[INVALID_TSS], invalid_tss);
    SET_IDT_ENTRY(idt[SEGMENT_NOT_PRESENT], segment_not_present);
    SET_IDT_ENTRY(idt[STACK_FAULT], stack_fault);
    SET_IDT_ENTRY(idt[GENERAL_PROTECTION], general_protection);
    SET_IDT_ENTRY(idt[PAGE_FAULT], page_fault);
    SET_IDT_ENTRY(idt[x87_FP_ERROR], x87_fp_error);
    SET_IDT_ENTRY(idt[ALIGNMENT_CHECK], alignment_check);
    SET_IDT_ENTRY(idt[MACHINE_CHECK], machine_check);
    SET_IDT_ENTRY(idt[SIMD_FP_ERROR], simd_fp_error);

    // Sets system call vector (x80) with corresponding function pointer
    SET_IDT_ENTRY(idt[SYSTEM_CALL_VECTOR], system_call_linkage);

    /* Interupts regarding the PIC. */
    idt[RTC].reserved3 = 0; // Need to change to interrupt gate. See ISA manual page 156.
    idt[RTC].present = 1;
    idt[KEYBOARD].reserved3 = 0; // Need to change to interrupt gate. See ISA manual page 156.
    idt[KEYBOARD].present = 1;
    idt[PIT].reserved3 = 0; // Need to change to interrupt gate. See ISA manual page 156.
    idt[PIT].present = 1;
    // Sets each interrupt with corresponding function pointer
    SET_IDT_ENTRY(idt[KEYBOARD], keyboard_handler_linkage);
    SET_IDT_ENTRY(idt[RTC], rtc_handler_linkage);
    SET_IDT_ENTRY(idt[PIT], pit_handler_linkage);
}

/* divide_error()
 * Inputs: none
 * Return Value: none
 * Function: Prints exception message and sends to infinite loop.
 */
void divide_error() {
    printf("Divide Error Exception\n");
    system_halt((uint8_t) EXCEPTION);
}

/* debug()
 * Inputs: none
 * Return Value: none
 * Function: Prints exception message and sends to infinite loop.
 */
void debug() {
    printf("Debug Exception\n");
    system_halt((uint8_t) EXCEPTION);
}

/* nmi()
 * Inputs: none
 * Return Value: none
 * Function: Prints exception message and sends to infinite loop.
 */
void nmi() {
    printf("NMI Interrupt\n");
    system_halt((uint8_t) EXCEPTION);
}

/* breakpoint()
 * Inputs: none
 * Return Value: none
 * Function: Prints exception message and sends to infinite loop.
 */
void breakpoint() {
    printf("Breakpoint Exception\n");
    system_halt((uint8_t) EXCEPTION);
}

/* overflow()
 * Inputs: none
 * Return Value: none
 * Function: Prints exception message and sends to infinite loop.
 */
void overflow() {
    printf("Overflow Exception\n");
    system_halt((uint8_t) EXCEPTION);
}

/* bound_range_exceeded()
 * Inputs: none
 * Return Value: none
 * Function: Prints exception message and sends to infinite loop.
 */
void bound_range_exceeded() {
    printf("BOUND Range Exceeded Exception\n");
    system_halt((uint8_t) EXCEPTION);
}

/* invalid_opcode()
 * Inputs: none
 * Return Value: none
 * Function: Prints exception message and sends to infinite loop.
 */
void invalid_opcode() {
    printf("Invalid Opcode Exception\n");
    system_halt((uint8_t) EXCEPTION);
}

/* device_not_available()
 * Inputs: none
 * Return Value: none
 * Function: Prints exception message and sends to infinite loop.
 */
void device_not_available() {
    printf("Device Not Available Exception\n");
    system_halt((uint8_t) EXCEPTION);
}

/* double_fault()
 * Inputs: none
 * Return Value: none
 * Function: Prints exception message and sends to infinite loop.
 */
void double_fault() {
    printf("Double Fault Exception\n");
    system_halt((uint8_t) EXCEPTION);
}

/* coprocessor_segment_overrun()
 * Inputs: none
 * Return Value: none
 * Function: Prints exception message and sends to infinite loop.
 */
void coprocessor_segment_overrun() {
    printf("Coprocessor Segment Overrun\n");
    system_halt((uint8_t) EXCEPTION);
}

/* invalid_tss()
 * Inputs: none
 * Return Value: none
 * Function: Prints exception message and sends to infinite loop.
 */
void invalid_tss() {
    printf("Invalid TSS Exception\n");
    system_halt((uint8_t) EXCEPTION);
}

/* segment_not_present()
 * Inputs: none
 * Return Value: none
 * Function: Prints exception message and sends to infinite loop.
 */
void segment_not_present() {
    printf("Segment Not Present\n");
    system_halt((uint8_t) EXCEPTION);
}

/* stack_fault()
 * Inputs: none
 * Return Value: none
 * Function: Prints exception message and sends to infinite loop.
 */
void stack_fault() {
    printf("Stack Fault Exception\n");
    system_halt((uint8_t) EXCEPTION);
}

/* general_protection()
 * Inputs: none
 * Return Value: none
 * Function: Prints exception message and sends to infinite loop.
 */
void general_protection() {
    printf("General Protection Exception\n");
    system_halt((uint8_t) EXCEPTION);
}

/* page_fault()
 * Inputs: none
 * Return Value: none
 * Function: Prints exception message and sends to infinite loop.
 */
void page_fault() {
    uint32_t location = page_fault_location();
    printf("Page-Fault Exception: %x \n", location);
    system_halt((uint8_t) EXCEPTION);
}

/* x87_fp_error()
 * Inputs: none
 * Return Value: none
 * Function: Prints exception message and sends to infinite loop.
 */
void x87_fp_error() {
    printf("x87 FPU Floating-Point Error\n");
    system_halt((uint8_t) EXCEPTION);
}

/* alignment_check()
 * Inputs: none
 * Return Value: none
 * Function: Prints exception message and sends to infinite loop.
 */
void alignment_check() {
    printf("Alignment Check Exception\n");
    system_halt((uint8_t) EXCEPTION);
}

/* machine_check()
 * Inputs: none
 * Return Value: none
 * Function: Prints exception message and sends to infinite loop.
 */
void machine_check() {
    printf("Machine-Check Exception\n");
    system_halt((uint8_t) EXCEPTION);
}

/* simd_fp_error()
 * Inputs: none
 * Return Value: none
 * Function: Prints exception message and sends to infinite loop.
 */
void simd_fp_error() {
    printf("SIMD Floating-Point Exception\n");
    system_halt((uint8_t) EXCEPTION);
}
