#define ASM     1

.text

.globl system_call_linkage
system_call_linkage:

    call *sys_call_table(, %eax, 4)
    iret

sys_call_table:
    .long system_halt, system_execute, system_read, system_write, system_open, system_close, system_getargs, system_vidmap, system_set_handler, system_sigreturn

