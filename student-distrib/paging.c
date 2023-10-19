/* paging.c - initializes paging stuff */

extern static inline void cr3_init(){
    asm volatile("MOVL $something, %cr3");
}


// The PDBR can then be changed either explicitly by loading a new value in CR3 with a MOV instruction or implicitly as part of a task switch.
//https://wiki.osdev.org/CPU_Registers_x86#CR3














/*CR0*/
// Bit	Label	Description
// 0	PE	Protected Mode Enable - set to 1, enables protected mode
// 1	MP	Monitor co-processor - 
// 2	EM	x87 FPU Emulation 
// 3	TS	Task switched
// 4	ET	Extension type
// 5	NE	Numeric error
// 16	WP	Write protect
// 18	AM	Alignment mask
// 29	NW	Not-write through
// 30	CD	Cache disable
// 31	PG	Paging - set to 1, enables paging

/*CR1*/
// Reserved, the CPU will throw a #UD exception when trying to access it.

/*CR2*/
// Bit	Label	Description
// 0-31 (63)	PFLA	Page Fault Linear Address - I believe when a page fault occurs the processor sets this, we dont set



/*CR3*/
// Bit	Label	Description	PAE	Long Mode
// 3	PWT	Page-level Write-Through	(Not used)	(Not used if bit 17 of CR4 is 1)
// 4	PCD	Page-level Cache Disable	(Not used)	(Not used if bit 17 of CR4 is 1)
// 12-31 (63)	PDBR	Page Directory Base Register	PAE: Base of PDPT	Long Mode: Base of PML4T/PML5T, I believe we are in PAE mode - set to not sure
// Bits 0-11 of the physical base address are assumed to be 0. Bits 3 and 4 of CR3 are only used when accessing a PDE in 32-bit paging without PAE.


/*CR4*/ 
// Bit	Label	Description
// 0	VME	Virtual 8086 Mode Extensions 
// 1	PVI	Protected-mode Virtual Interrupts 
// 2	TSD	Time Stamp Disable
// 3	DE	Debugging Extensions 
// 4	PSE	Page Size Extension - set to 1, Enables 4-MByte pages when set
// 5	PAE	Physical Address Extension - set to 1, Enables paging mechanism to reference 36-bit physical addresses when set
// 6	MCE	Machine Check Exception - 
// 7	PGE	Page Global Enabled - set to 1, Enables the global page feature when set; disables the global page feature when clear. 
// 8	PCE	Performance-Monitoring Counter enable
// 9	OSFXSR	Operating system support for FXSAVE and FXRSTOR instructions
// 10	OSXMMEXCPT	Operating System Support for Unmasked SIMD Floating-Point Exceptions
// 11	UMIP	User-Mode Instruction Prevention (if set, #GP on SGDT, SIDT, SLDT, SMSW, and STR instructions when CPL > 0)
// 13	VMXE	Virtual Machine Extensions Enable
// 14	SMXE	Safer Mode Extensions Enable
// 16	FSGSBASE	Enables the instructions RDFSBASE, RDGSBASE, WRFSBASE, and WRGSBASE
// 17	PCIDE	PCID Enable
// 18	OSXSAVE	XSAVE and Processor Extended States Enable
// 20	SMEP	Supervisor Mode Execution Protection Enable
// 21	SMAP	Supervisor Mode Access Prevention Enable
// 22	PKE	Protection Key Enable
// 23	CET	Control-flow Enforcement Technology
// 24	PKS	Enable Protection Keys for Supervisor-Mode Pages

