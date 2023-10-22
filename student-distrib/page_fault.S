
.text

.global page_fault_location

.align 4
page_fault_location:
        pushl   %ebp
        movl    %esp, %ebp

        # return page fault lcoation

        movl    %cr2, %eax

        leave
        ret