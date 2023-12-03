#ifndef IDT_H
#define IDT_H

#include "interrupt_link.h"
#include "x86_desc.h"
#include "lib.h"

#define NUM_EXCEPTIONS  20

/* Interrupts and corresponding vector number. */
#define KEYBOARD   0x21
#define RTC        0x28
#define PIT        0x20

/* Vector number for system calls. */
#define SYSTEM_CALL_VECTOR    0x80

/* Exceptions with corresponding vector number. */
#define DIVIDE_ERROR    0
#define DEBUG           1
#define NMI             2
#define BREAKPOINT      3
#define OVERFLOW        4
#define BOUND_RANGE_EXCEEDED    5
#define INVALID_OPCODE  6
#define DEVICE_NOT_AVAILABLE    7
#define DOUBLE_FAULT    8
#define COPROCESSOR_SEGMENT_OVERRUN 9
#define INVALID_TSS     10
#define SEGMENT_NOT_PRESENT     11
#define STACK_FAULT     12
#define GENERAL_PROTECTION      13
#define PAGE_FAULT      14
#define SKIP            15
#define x87_FP_ERROR    16
#define ALIGNMENT_CHECK 17
#define MACHINE_CHECK   18
#define SIMD_FP_ERROR   19

/* Initialize the idt */
void idt_init();

/* Build idt */
void build_idt();

/* Functions for each exception. Blue screen of death. */
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

/* Function for system calls. Will be changed in future checkpoints. */
// void system_call();

extern uint32_t page_fault_location();

#endif /* IDT_H */

