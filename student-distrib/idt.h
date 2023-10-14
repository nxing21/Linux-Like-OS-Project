#ifndef IDT_H
#define IDT_H

#define IDT_LENGTH  0x14    /* length of idt */

/* Initialize the idt */
void idt_init();

/* Build idt */
void build_idt();


/*exception handler function pointer*/
//https://stackoverflow.com/questions/5309859/how-to-define-an-array-of-functions-in-c
typedef void (*exception_handler_t)();  //void just means the type is not defined, () means args undefined
exception_handler_t exception_handler[256] = {&divide_error, &debug, &nmi, &breakpoint, &overflow, &bound_range_exceeded, &invalid_opcode, &device_not_available, &double_fault, &coprocessor_segment_overrun, 
&invalid_tss, &segment_not_present, &stack_fault, &general_protection, &page_fault, NULL, &x87_fp_error, &alignment_check, &machin_check, &simd_fp_error};



void divide_error();
void debug();
void nmi();
void breakpoint(); 
void overflow(); 
void bound_range_exceeded(); 
void invalid_opcode(); 
void device_not_available(); 
void double_fault(); 
void coprocessor_segment_overrun(); 
void invalid_tss(); 
void segment_not_present(); 
void stack_fault(); 
void general_protection();
void page_fault();
void x87_fp_error();
void alignment_check();
void machine_check();
void simd_fp_error();






// extern int32_t system_call_handler(uint8_t arg, int32_t fd, int32_t nbytes, void* address);
// /*system calls*/
// static inline int32_t halt (uint8_t status);
// static inline int32_t execute (const uint8_t* command);
// static inline int32_t read (int32_t fd, void* buf, int32_t nbytes);
// static inline int32_t write (int32_t fd, const void* buf, int32_t nbytes);
// static inline int32_t close (int32_t fd);
// static inline int32_t open (const uint8_t* filename);
// static inline int32_t getargs (uint8_t* buf, int32_t nbytes);
// static inline int32_t vidmap (uint8_t** screen_start);
// static inline int32_t set_handler (int32_t signum, void* handler_address);
// static inline int32_t sigreturn (void);

#endif /* IDT_H */

