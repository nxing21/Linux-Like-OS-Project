/* idt.c - Everything to do with the idt
 */

#include "idt.h"
#include "x86_desc.h"
#include "lib.h"



/* Initialize the idt */
void idt_init() {
    // build idt
    build_idt();
    // lidt macro
    lidt(idt_desc_ptr);
}

void build_idt() {
    int i;

    for (i = 0; i < NUM_VEC; i++) {
        idt[i].seg_selector = KERNEL_CS;
        idt[i].reserved4 = 0;
        idt[i].reserved3 = 1;
        idt[i].reserved2 = 1;
        idt[i].reserved1 = 1;
        idt[i].size = 1;
        idt[i].reserved0 = 0;
        idt[i].dpl = 0;
        idt[i].present = 0;
        if (i < NUM_EXCEPTIONS) {
            idt[i].present = 1;
        }
    }

    // for offset, we need to call SET_IDT_ENTRY
    SET_IDT_ENTRY(idt[0], &divide_error);
    SET_IDT_ENTRY(idt[1], &debug);
    SET_IDT_ENTRY(idt[2], &nmi);
    SET_IDT_ENTRY(idt[3], &breakpoint);
    SET_IDT_ENTRY(idt[4], &overflow);
    SET_IDT_ENTRY(idt[5], &bound_range_exceeded);
    SET_IDT_ENTRY(idt[6], &invalid_opcode);
    SET_IDT_ENTRY(idt[7], &device_not_available);
    SET_IDT_ENTRY(idt[8], &double_fault);
    SET_IDT_ENTRY(idt[9], &coprocessor_segment_overrun);
    SET_IDT_ENTRY(idt[10], &invalid_tss);
    SET_IDT_ENTRY(idt[11], &segment_not_present);
    SET_IDT_ENTRY(idt[12], &stack_fault);
    SET_IDT_ENTRY(idt[13], &general_protection);
    SET_IDT_ENTRY(idt[14], &page_fault);
    SET_IDT_ENTRY(idt[16], &x87_fp_error);
    SET_IDT_ENTRY(idt[17], &alignment_check);
    SET_IDT_ENTRY(idt[18], &machine_check);
    SET_IDT_ENTRY(idt[19], &simd_fp_error);
    
        //128 = x80 which is int for system calls 
        // idt[128].seg_selector = KERNEL_CS;
        // idt[128].reserved4 = 0;
        // idt[128].reserved3 = 1; //changed this bc it uses trap gate?
        // idt[128].reserved2 = 1;
        // idt[128].reserved1 = 1;
        // idt[128].size = 1; // size of gate: 1 = 32bits, 0 = 16bits
        // idt[128].reserved0 = 0;
        // idt[128].dpl = 0; // kernel level, descriptor privilege level
        // idt[128].present = 1; // segment present flag

        // SET_IDT_ENTRY(idt[128], &system_call_handler);



}


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



//idt[128], system call handler


// int32_t system_call_handler(uint8_t arg, int32_t fd, int32_t nbytes, void* address){
//     switch(/*something*/){
//         case /*something*/:
//             return halt(arg);
//         case /*something*/:
//             return execute((const)arg);
//         case /*something*/:
//             return read (fd, (void*)arg, nbytes);
//         case /*something*/:
//             return write (fd, (const void*)arg, nbytes);
//         case /*something*/:
//             return close (fd);
//         case /*something*/:
//             return open ((const uint8_t*) arg);
//         case /*something*/:
//             return getargs ((const uint8_t*) arg, nbytes);
//         case /*something*/:
//             return vidmap ((uint8_t**) arg);
//         case /*something*/:
//             return set_handler (nbytes /*probably bad practice but the type fits*/, address);
//         case /*something*/:
//             return sigreturn ();
//         default:
//             return -1; //failed call is -1
        
//     }

// };

void divide_error() {
    printf("Divide Error Exception\n");
    while(1){};
}
void debug() {
    printf("Debug Exception\n");
    while(1){};
}
void nmi() {
    printf("NMI Interrupt\n");
    while(1){};
}
void breakpoint() {
    printf("Breakpoint Exception\n");
    while(1){};
}
void overflow() {
    printf("Overflow Exception\n");
    while(1){};
}
void bound_range_exceeded() {
    printf("BOUND Range Exceeded Exception\n");
    while(1){};
}
void invalid_opcode() {
    printf("Invalid Opcode Exception\n");
    while(1){};
}
void device_not_available() {
    printf("Device Not Available Exception\n");
    while(1){};
}
void double_fault() {
    printf("Double Fault Exception\n");
    while(1){};
}
void coprocessor_segment_overrun() {
    printf("Coprocessor Segment Overrun\n");
    while(1){};
}
void invalid_tss() {
    printf("Invalid TSS Exception\n");
    while(1){};
}
void segment_not_present() {
    printf("Segment Not Present\n");
    while(1){};
}
void stack_fault() {
    printf("Stack Fault Exception\n");
    while(1){};
}
void general_protection() {
    printf("General Protection Exception\n");
    while(1){};
}
void page_fault() {
    printf("Page-Fault Exception\n");
    while(1){};
}
void x87_fp_error() {
    printf("x87 FPU Floating-Point Error\n");
    while(1){};
}
void alignment_check() {
    printf("Alignment Check Exception\n");
    while(1){};
}
void machine_check() {
    printf("Machine-Check Exception\n");
    while(1){};
}
void simd_fp_error() {
    printf("SIMD Floating-Point Exception\n");
    while(1){};
}

/*system calls*/
// static inline int32_t halt (uint8_t status){};
// static inline int32_t execute (const uint8_t* command){};
// static inline int32_t read (int32_t fd, void* buf, int32_t nbytes){};
// static inline int32_t write (int32_t fd, const void* buf, int32_t nbytes){};
// static inline int32_t close (int32_t fd){};
// static inline int32_t open (const uint8_t* filename){};
// static inline int32_t getargs (uint8_t* buf, int32_t nbytes){};
// static inline int32_t vidmap (uint8_t** screen_start){};
// static inline int32_t set_handler (int32_t signum, void* handler_address){};
// static inline int32_t sigreturn (void){};