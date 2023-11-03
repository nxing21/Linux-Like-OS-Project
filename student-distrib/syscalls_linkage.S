#define ASM     1

.data
    NUM_SYS_CALLS = 10

.text

# system_call_linkage
# Inputs: %eax holds the system call number we want to execute
# Return Value: none
# Function: Calls the corresponding system call, while saving registers, or does nothing if invalid number
.globl system_call_linkage
.align 4
system_call_linkage:
    # Save registers
    pushfl
    pushl %esp
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx

    # Check if valid
    cmpl    $0, %eax
    jle invalid_number
    cmpl    $NUM_SYS_CALLS, %eax
    jg invalid_number
    
    # Push arguments
    pushl %edx
    pushl %ecx
    pushl %ebx
    call *sys_call_table(, %eax, 4) # Call the corresponding system call (4 bytes per function pointer)
    addl $12, %esp # Pop arguments
    jmp finished

invalid_number:
    movl $-1, %eax
finished:
    # Restore registers
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp
    popl %esp
    popfl
    iret

# Jump table (includes the 10 system calls we are implementing for this MP)
sys_call_table:
    .long 0, system_halt, system_execute, system_read, system_write, system_open, system_close, system_getargs, system_vidmap, system_set_handler, system_sigreturn

