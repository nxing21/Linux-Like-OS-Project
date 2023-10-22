#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "file_sys.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test() {
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/* Divide by Zero Exception Test
 * Inputs: None
 * Outputs: FAIL on fail
 * Side Effects: None
 * Coverage: Load IDT, IDT definition of Interrupt 0—Divide Error Exception
 */
int divide_error_test() {
	TEST_HEADER;

	int a;
	int b;
	b = 0;
	a = 2 / b; // 2/0 should trigger a div by zero exception
	return FAIL; /*shouldn't reach here unless test failed*/ 
}

/* BOUND Range Exceeded Exception Test
 * Inputs: None
 * Outputs: FAIL on fail
 * Side Effects: None
 * Coverage: Load IDT, IDT definition of Interrupt 5—BOUND Range Exceeded Exception
 */
static inline int boundrange_error_test(){
	TEST_HEADER;
	asm volatile("int $5"); //interrupt #5 maps to BOUND Range Exceeded Exception
	return FAIL; /*shouldn't reach here unless test failed*/ 
}


/* Page Fault Test (using zero pointer)
 * Inputs: None
 * Outputs: FAIL on fail
 * Side Effects: None
 * Coverage: Load IDT, IDT definition of Interrupt 14—Page-Fault Exception and Paging implementation
 */
int page_fault_zero_test(){
	TEST_HEADER;
	
	int* bad_ptr = (int*)(0x0); // pointer pointing to memory that shouldn't be accessed
	int test_value;
	test_value = *(bad_ptr);
	return FAIL; /*shouldn't reach here unless test failed*/ 
}

/* Page Fault Test (using null pointer)
 * Inputs: None
 * Outputs: FAIL on fail
 * Side Effects: None
 * Coverage: Load IDT, IDT definition of Interrupt 14—Page-Fault Exception and Paging implementation
 */
int page_fault_null_test() {
	TEST_HEADER;
	int *p = NULL; 
	*p = 1; // dereferencing a null ptr should cause a page fault
	return FAIL; /*shouldn't reach here unless test failed*/ 
}

/* Page Fault Test (using pointer greater than 8 MB)
 * Inputs: None
 * Outputs: FAIL on fail
 * Side Effects: None
 * Coverage: Load IDT, IDT definition of Interrupt 14—Page-Fault Exception and Paging implementation
 */
int page_fault_big_test() {
	TEST_HEADER;
	uint32_t kernel_start_addr = 0x800000;
	int* ptr = (int*)(kernel_start_addr + 8); // pointer pointing to memory that shouldn't be accessed
	int test_value;
	test_value = *(ptr);

	return FAIL; /*shouldn't reach here unless test failed*/ 
}

/* Page Fault Test (using out of pointer less than 4MB)
 * Inputs: None
 * Outputs: FAIL on fail
 * Side Effects: None
 * Coverage: Load IDT, IDT definition of Interrupt 14—Page-Fault Exception and Paging implementation
 */
int page_fault_too_small_test() {
	TEST_HEADER;
	uint32_t kernel_addr = 0x400000;
	int* ptr = (int*)(kernel_addr - 8); // pointer pointing to memory that shouldn't be accessed
	int test_value;
	test_value = *(ptr);

	return FAIL; /*shouldn't reach here unless test failed*/ 
}

/* Page Test (using random good pointer)
 * Inputs: None
 * Outputs: PASS on pass
 * Side Effects: None
 * Coverage: Paging implementation
 */
int page_test(){
	TEST_HEADER;
	
	int good_ptr = 1024; // random num
	int * lol2;
	lol2 = &good_ptr;

	return PASS; /*should always reach here unless test failed*/ 
}

/* Page Video Memory Test 
 * Inputs: None
 * Outputs: PASS on pass
 * Side Effects: None
 * Coverage: Paging implementation specifically for video memory
 */
int page_videomem_test(){
	TEST_HEADER;
	uint32_t vid_addr = 0xB8000;
	int* videomem_ptr = (int*)(vid_addr + 1); // ptr to address within video mem
	int lol2;
	lol2 = *videomem_ptr;

	return PASS; /*should always reach here unless test failed*/ 
}

/* Page Kernel Memory Test 
 * Inputs: None
 * Outputs: PASS on pass
 * Side Effects: None
 * Coverage: Paging implementation specifically for kernel memory
 */
int page_kernelmem_test(){
	TEST_HEADER;
	uint32_t kernel_addr = 0x400000;
	int* kernelmem_ptr = (int*)(kernel_addr + 1); // ptr to address within kernel mem
	int lol2;
	lol2 = *kernelmem_ptr;

	return PASS; /*should always reach here unless test failed*/ 
}


/* System Call Test
 * Inputs: None
 * Outputs: FAIL on fail
 * Side Effects: None
 * Coverage: Load IDT, IDT definition of Interrupt x80 - System Calls
 */
static inline int sys_call_test(){
	TEST_HEADER;
	asm volatile("int $128"); //interrupt #0x80 (= #128) which maps to system calls
	return FAIL; /*shouldn't reach here unless test failed*/ 
}


/* Checkpoint 2 tests */

/* read_dentry_test
 * Inputs: None
 * Outputs: PASS on pass
 * Side Effects: None
 * Coverage: check if read dentry funcs works
 */
int read_dentry_test(){
	clear();
	TEST_HEADER;
	dentry_t* dentry;
	const int8_t* fname = "verylargetextwithverylongname.txt";
	int32_t out = read_dentry_by_name((const uint8_t *) fname, dentry);
	if(out != 0){
		return FAIL;
	}
	
	printf("%s %d  lol1 \n", dentry->filename, dentry->inode_num);


	if(read_dentry_by_index (1, dentry) != 0){
		
		return FAIL;
	}
	
	printf("%s %d  lol2 \n", dentry->filename, dentry->inode_num);

	return PASS; /*should always reach here unless test failed*/ 

}

/* Checkpoint 2 tests */

/* read_data_test
 * Inputs: None
 * Outputs: PASS on pass
 * Side Effects: None
 * Coverage: check if read data works
 */
int read_data_test(){
	//clear();
	TEST_HEADER;
	uint8_t buf[4 * BYTES_PER_BLOCK];
	int32_t bytes_read;
	uint32_t inode_num = 38;
	bytes_read = read_data(inode_num, 0, buf, 4 * BYTES_PER_BLOCK);
	clear();
	int i;
	for (i = 0; i < bytes_read; i++) {
		printf("%c", buf[i]);
	}

	return PASS; /*should always reach here unless test failed*/ 
}

int read_dir_test() {
	TEST_HEADER;
	int32_t fd;
	uint8_t buf[FILENAME_LEN];
	read_directory(fd, buf, 0);

	return PASS;
}

int read_file_test() {
	TEST_HEADER;
	int32_t fd;
	uint8_t buf[BYTES_PER_BLOCK * 4];
	read_file(fd, buf, 0);

	return PASS;
}

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	// TEST_OUTPUT("idt_test", idt_test());
	// launch your tests here
	// TEST_OUTPUT("divide_error_test", divide_error_test());
	// TEST_OUTPUT("page_fault_zero_test", page_fault_zero_test());
	// TEST_OUTPUT("page_test", page_test());
	// TEST_OUTPUT("page_fault_big_test", page_fault_big_test());
	// TEST_OUTPUT("page_fault_null_test", page_fault_null_test());
	// TEST_OUTPUT("boundrange_error_test", boundrange_error_test());
	// TEST_OUTPUT("page_videomem_test", page_videomem_test());
	// TEST_OUTPUT("page_fault_too_small_test", page_fault_too_small_test());
	// TEST_OUTPUT("page_kernelmem_test", page_kernelmem_test());

	/* Checkpoint 2 tests*/
	// TEST_OUTPUT("read_dentry_test", read_dentry_test());
	// TEST_OUTPUT("read_dir_test", read_dir_test());
	TEST_OUTPUT("read_file_test", read_file_test());
	
	
	
}

