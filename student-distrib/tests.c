#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "file_sys.h"
#include "rtc.h"
#include "terminal.h"
#include "syscalls.h"

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

int32_t RTC_read(int32_t fd, void* buffer, int32_t nbytes);
int32_t RTC_write(int32_t fd, const void* buffer, int32_t nbytes);

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
	// clear();
	TEST_HEADER;
	dentry_t dentry;
	const char* fname = "verylargetextwithverylongname.tx";
	int32_t out = read_dentry_by_name((const uint8_t *) fname, &dentry);
	if(out != 0){
		return FAIL;
	}
	
	printf("Filename: %s Inode Number: %d \n", dentry.filename, dentry.inode_num);


	// if(read_dentry_by_index (1, dentry) != 0){
		
	// 	return FAIL;
	// }
	
	// printf("%s %d  lol2 \n", dentry->filename, dentry->inode_num);

	return PASS; /*should always reach here unless test failed*/ 

}


/* read_data_test
 * Inputs: None
 * Outputs: PASS on pass
 * Side Effects: None
 * Coverage: check if read data works
 */
int read_data_test(){
		TEST_HEADER;
	uint8_t buf[BYTES_PER_BLOCK*4];
		int32_t bytes_read;
	uint32_t inode_num = 38;
	bytes_read = read_data(inode_num, 0, buf, 187);
	// clear();
	int i;
	for (i = 0; i < bytes_read; i++) {
		putc(buf[i]);
	}

	return PASS; /*should always reach here unless test failed*/ 
}



/* open_file_test
 * Inputs: None
 * Outputs: PASS on pass
 * Side Effects: None
 * Coverage: check if open file works
 */
int open_read_file_test(){
	TEST_HEADER;
	// clear();
	const char* filename =  "cat";
	uint8_t buf[10000]; // arbitrary big number
	int i;
	for (i = 0; i < 10000; i++) {
		buf[i] = 0x00;
	}
	if(open_file((const uint8_t *) filename) == -1){
		printf("failed at open");
		return FAIL;
	}

	int bytes_read = read_file(3, buf, 10000); // arbitrary big number
	for (i = 0; i < bytes_read; i++) {
		if (buf[i] == '\0') {
			continue;
		}
		printf("%c", buf[i]);
	}
	printf("\n");

	printf("\n");

	return PASS; /*should always reach here unless test failed*/ 

}

/* open_read_dir_test()
 * Inputs: None
 * Outputs: PASS on pass
 * Side Effects: None
 * Coverage: check if open directory works
 */
int open_read_dir_test(){
	TEST_HEADER;
	// clear();
	const char* filename =  ".";
	uint8_t buf[BYTES_PER_BLOCK*4];
	uint32_t length_buf[BYTES_PER_BLOCK];
	if(open_file((const uint8_t *) filename) == -1){
		printf("failed at open");
		return FAIL;
	}
	
	int num_read = read_directory(3, buf, 1000);
	int i;
	int length_index = 0;
	for (i = 0; i < num_read; i++) {
		if (i % (FILENAME_LEN + 1) == 0) { // one more than the file length for the file type character
			printf("\nFile name: ");
			putc(buf[i]);
		}
		else if (i % (FILENAME_LEN + 1) == FILENAME_LEN) {
			printf(", File type: ");
			putc(buf[i]);
			printf(", File size: ");
			printf("%d", length_buf[length_index]);
			length_index++;
		}
		else {
			putc(buf[i]);
		}
	}

	printf("\n");

	return PASS; /*should always reach here unless test failed*/ 
}

/* RTC_frequencies_test()
 * Inputs: None
 * Outputs: PASS on pass
 * Side Effects: None
 * Coverage: check if valid frequencies work
 */
int RTC_frequencies_test(){
	TEST_HEADER;
	int i, k, temp;

	for (i = 0; i <= 9; i++) {
		temp = 2 << i;

		if (temp >= 1000) {
			printf("%d Hz: ", temp);
		} else if (temp >= 100) {
			printf(" %d Hz: ", temp);
		} else if (temp >= 10) {
			printf("  %d Hz: ", temp);
		} else {
			printf("   %d Hz: ", temp);
		}

		RTC_write(0,&temp,4);
		for (k = 0; k < 8; k++) {
			printf("o ");
			RTC_read(0, 0, 4);
		}
		printf("\n");
	}
	return PASS;
}

/* RTC_frequencies_low_test()
 * Inputs: None
 * Outputs: PASS on pass
 * Side Effects: None
 * Coverage: check if low invalid frequencies don't work
 */
int RTC_frequencies_low_test() {
	TEST_HEADER;
	int i, temp;

	temp = 1024;
	RTC_write(0,&temp,4);
	temp = 1;
	RTC_write(0,&temp,4);
	printf("Trying 1 Hz\n");
	printf("Should be 1024 Hz: ");
	for (i = 0; i < 8; i++) {
		printf("o ");
		RTC_read(0,0,4);
	}
	printf("\n");

	return PASS;
}

/* RTC_frequencies_high_test()
 * Inputs: None
 * Outputs: PASS on pass
 * Side Effects: None
 * Coverage: check if high invalid frequencies don't work
 */
int RTC_frequencies_high_test() {
	TEST_HEADER;
	int i, temp;

	temp = 2;
	RTC_write(0,&temp,4);
	temp = 2048;
	RTC_write(0,&temp,4);
	printf("Trying 2048 Hz\n");
	printf("Should be 2 Hz: ");
	for (i = 0; i < 8; i++) {
		printf("o ");
		RTC_read(0,0,4);
	}
	printf("\n");

	return PASS;
}

/* RTC_frequencies_invalid_test()
 * Inputs: None
 * Outputs: PASS on pass
 * Side Effects: None
 * Coverage: check if invalid frequencies don't work
 */
 int RTC_frequencies_invalid_test() {
	TEST_HEADER;
	int i, k, temp;

	temp = 4;
	RTC_write(0,&temp,4);
	printf("Should stay at 4 Hz\n");

	for (i = 1; i < 9; i++) {
		printf("Trying new invalid freq: ");
		temp = i*100;
		RTC_write(0,&temp,4);
		for (k = 0; k < 4; k++) {
			printf("o ");
			RTC_read(0,0,4);
		}
		printf("\n");
	}

	return PASS;
}

/* RTC_open_close_test()
 * Inputs: None
 * Outputs: PASS on correct close(); FAIL on else
 * Side Effects: None
 * Coverage: check if open() and close() work
 */
int RTC_open_close_test() {
	TEST_HEADER;
	int i;
	uint8_t temp = 0;
	RTC_open(&temp);

	printf("Should be 2 Hz: ");
	for (i = 0; i < 8; i++) {
		printf("o ");
		RTC_read(0,0,4);
	}
	printf("\n");

	if (RTC_close(0) == 0) {
		return PASS;
	}
	return FAIL;
}

/* test_terminal_open_close()
 * Inputs: None
 * Outputs: PASS on correct close(); FAIL on else
 * Side Effects: None
 * Coverage: check if open() and close() work
 */
int test_terminal_open_close(){
	uint8_t buf[128];
	int i;
	int numbytes;

	if (terminal_open("BRUH") == 0){
		printf("Terminal open.\n");
	for (i = 0; i < 4; i++){
	// while(){
		numbytes = terminal_read(0, buf, 128);
		if (numbytes != -1){
			terminal_write(0, buf, numbytes);
		}
	}
	if (terminal_close(0) == 0){
		printf("Terminal close.\n");
		return PASS;
	}
	}
	return FAIL;
}

/* test_terminal_read_write()
 * Inputs: None
 * Outputs: None
 * Side Effects: Will put the OS in a while loop to test read/write of terminal.
 * Coverage: check if read/write works;
 */
void test_terminal_read_write(){
	uint8_t buf[128];
	uint8_t edit_buf[128];
	int numbytes;

	edit_buf[0] = 'H';
	edit_buf[1] = 'e';
	edit_buf[2] = 'l';
	edit_buf[3] = 'l';
	edit_buf[4] = 'o';
	edit_buf[5] = ' ';

	numbytes = terminal_read(0, buf, 128);
	if (numbytes != -1){
			terminal_write(0, buf, numbytes);
	}
	/* This part tests if anything that is already in the buffer will be editted. USE CTRL-L*/
	numbytes = terminal_read(0, edit_buf + 6, 128);
	if (numbytes != -1){
			terminal_write(0, edit_buf, numbytes+6);
	}
	while (1){
		numbytes = terminal_read(0, buf, 128);
		if (numbytes != -1){
			terminal_write(0, buf, numbytes);
		}
	}
}
/* Checkpoint 3 tests */


// /* sys_open_read_file_test
//  * Inputs: None
//  * Outputs: PASS on pass
//  * Side Effects: None
//  * Coverage: check if sys open file works
//  */
// int sys_open_read_file_test(){
// 	TEST_HEADER;
// 	// clear();
// 	const char* filename =  "shell";
// 	uint8_t buf[10000]; // arbitrary big number
// 	int i;
// 	for (i = 0; i < 10000; i++) {
// 		buf[i] = 0x00;
// 	}
// 	if(system_open((const uint8_t *) filename) == -1){
// 		printf("failed at open");
// 		return FAIL;
// 	}

// 	int bytes_read = read_file(2, buf, 10000); // arbitrary big number
// 	for (i = 0; i < bytes_read; i++) {
// 		if (buf[i] == '\0') {
// 			continue;
// 		}
// 		printf("%c", buf[i]);
// 	}
// 	printf("\n");

// 	printf("\n");

// 	return PASS; /*should always reach here unless test failed*/ 

// }

// /* sys_open_read_dir_test()
//  * Inputs: None
//  * Outputs: PASS on pass
//  * Side Effects: None
//  * Coverage: check if open directory works
//  */
// int sys_open_read_dir_test(){
// 	TEST_HEADER;
// 	// clear();
// 	// curr_fds[2].file_pos = 0;
// 	// curr_fds[2].flags = -1;
// 	// curr_fds[2].inode = 0;
// 	const char* filename =  ".";
// 	uint8_t buf[BYTES_PER_BLOCK*4];
// 	if(system_open((const uint8_t *) filename) == -1){
// 		printf("failed at open");
// 		return FAIL;
// 	}
	
// 	int num_read = read_directory(2, buf, 1000);
// 	int i;
// 	int length_index = 0;
// 	for (i = 0; i < num_read; i++) {
// 		if (i % (FILENAME_LEN + 1) == 0) { // one more than the file length for the file type character
// 			printf("\nFile name: ");
// 			putc(buf[i]);
// 		}
// 		else if (i % (FILENAME_LEN + 1) == FILENAME_LEN) {
// 			printf(", File type: ");
// 			putc(buf[i]);
// 			printf(", File size: ");
// 			printf("%d", length_buf[length_index]);
// 			length_index++;
// 		}
// 		else {
// 			putc(buf[i]);
// 		}
// 	}

// 	printf("\n");

// 	return PASS; /*should always reach here unless test failed*/ 
// }

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
	TEST_OUTPUT("read_dentry_test", read_dentry_test());
	// TEST_OUTPUT("read_data_test", read_data_test());
	// TEST_OUTPUT("open_read_file_test", open_read_file_test());
	// test_terminal_read_write(); /* Comment this out if you want to separately test read/write of terminal. */
	// TEST_OUTPUT("test_terminal_read_write", test_terminal_open_close());
	
	// TEST_OUTPUT("RTC_frequencies_test", RTC_frequencies_test());
	// TEST_OUTPUT("RTC_frequencies_low_test", RTC_frequencies_low_test());
	// TEST_OUTPUT("RTC_frequencies_high_test", RTC_frequencies_high_test());
	// TEST_OUTPUT("RTC_frequencies_invalid_test", RTC_frequencies_invalid_test());
	// TEST_OUTPUT("RTC_open_close_test", RTC_open_close_test());

	
	
	
}

