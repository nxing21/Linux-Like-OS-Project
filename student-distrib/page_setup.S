#define ASM     1
#include "page.h"

.text

.global loadPageDirectory
.global enablePaging

# https://wiki.osdev.org/Setting_Up_Paging#:~:text=Advanced%20Paging%20Example-,Basic%20Paging,to%20find%20the%20page%20tables.

# places address into CR3
.align 4
loadPageDirectory:
        pushl   %ebp
        movl    %esp, %ebp

        movl    8(%ebp), %eax
        movl    %eax, %cr3

        movl    %cr4, %eax
        orl     $0x00000010, %eax
        movl	%eax, %cr4

        leave
        ret

# enables paging
.align 4
enablePaging:
        pushl   %ebp
        movl    %esp, %ebp

        movl    %cr0, %eax
        orl     $0x80000001, %eax
        movl    %eax, %cr0

        leave
        ret

