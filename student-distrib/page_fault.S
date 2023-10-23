
.text

.global page_fault_location

# int page_fault_location(void)
# Tasks:
#	(1) return location of page fault
# Inputs	: none
# Outputs	: int - page fault location
.align 4
page_fault_location:
        pushl   %ebp
        movl    %esp, %ebp

        # return page fault location

        movl    %cr2, %eax

        leave
        ret
