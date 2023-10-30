#define ASM     1
#include "page.h"


.text

.global loadPageDirectory   # loads page directory
.global enablePaging   # enables paging
.global flushTLB   # flushes TLBs


# int loadPageDirectory(page_directory*)
# Tasks:
#	(1) load cr3 with page directory address
#       (2) set page size extensions in cr4
# Inputs	: page_directory* -- points to the page directory
# Outputs	: int - ignore
.align 4
loadPageDirectory:
        pushl   %ebp
        movl    %esp, %ebp

        # load cr3 with page directory address
        movl    8(%ebp), %eax
        movl    %eax, %cr3

        # set page size extensions in cr4
        movl    %cr4, %eax
        orl     $0x00000010, %eax
        movl	%eax, %cr4

        leave
        ret


# int enablePaging(void)
# Tasks:
#	(1) enable paging and protection in cr0
# Inputs	: none
# Outputs	: int - ignore
.align 4
enablePaging:
        pushl   %ebp
        movl    %esp, %ebp

        movl    %cr0, %eax
        orl     $0x80000001, %eax
        movl    %eax, %cr0

        leave
        ret


# int flushTLB(void)
# Tasks:
#	(1) flush TLBs
# Inputs	: none
# Outputs	: int - ignore
.align 4
flushTLB:
        movl    %cr3, %eax
        movl    %eax, %cr3
        
        ret

