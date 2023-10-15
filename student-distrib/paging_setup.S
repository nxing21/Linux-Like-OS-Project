#define ASM     1
#include "paging.h"

.global setup

setup:
        pushl   %ebp
        movl    %esp, %ebp
        pushl   %esi
        pushl   %edi
        pushl   %ebx

        # page enable (bit 31 in CR0)
        movl    %cr0, %esi
        orl     $0x80000001, %esi
        movl    %esi, %cr0

        # page size extension (bit 4 in CR4)
        movl    %cr4, %esi
        orl     $0x10, %esi
        movl    %esi, %cr4

        # set cr3 to page directory address

        popl    %ebx
        popl    %edi
        popl    %esi
        leave
        ret

