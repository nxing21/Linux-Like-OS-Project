#define ASM     1

.data
    NUM_SYS_CALLS = 6

.text

.globl system_call_linkage
.align 4
system_call_linkage:
    # Save registers
    pushf
    pushl %ebp
    pushl %edi
    pushl %esi

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
    popl %ebx
    popl %ecx
    popl %edx
    jmp finished

invalid_number:
    movl $-1, %eax
finished:
    # Restore registers
    popl %esi
    popl %edi
    popl %ebp
    popf
    iret

# Jump table
sys_call_table:
    .long 0, system_halt, system_execute, system_read, system_write, system_open, system_close

