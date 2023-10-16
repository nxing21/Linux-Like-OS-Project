#include "tests.h"
#include "x86_desc.h"
#include "lib.h"

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

int divide_error_test() {
	TEST_HEADER;

	int a;
	int b;
	b = 0;
	a = 2 / b;
	return FAIL;
}

int overflow_error_test() {
	TEST_HEADER;

	int a[3];
	int x;
	x = a[0];
	
	return FAIL;
}

static inline int boundrange_error_test(){
	TEST_HEADER;
	asm volatile("int $5");
	return FAIL;
}

// // doesn't work
// int overflow_error_test() {
// 	TEST_HEADER;

// 	int32_t a;
// 	a = -2147483648;
// 	a *= -10;
// 	return FAIL;
// }


// add more tests here
int page_fault_zero_test(){
	TEST_HEADER;
	
	int* bad_ptr = (int*)(0x0);
	int test_value;
	test_value = *(bad_ptr);
	// int *lol1;
	// lol1 = &bad_ptr;

	return FAIL;
}

int page_fault_null_test() {
	TEST_HEADER;
	// int* ptr = (int*)(0x800000 + 8);
	// int test_value = *(ptr);

	int *p = NULL;
	*p = 1;
	return FAIL;
}

int page_fault_big_test() {
	TEST_HEADER;

	int* ptr = (int*)(0x800000 + 8);
	int test_value;
	test_value = *(ptr);

	return FAIL;
}


int page_test(){
	TEST_HEADER;
	
	int good_ptr = 1024;
	int * lol2;
	lol2 = &good_ptr;

	return PASS;
}

static inline int sys_call_test(){
	TEST_HEADER;
	asm volatile("int $128");
	return FAIL;
}


/* Checkpoint 2 tests */
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	// TEST_OUTPUT("idt_test", idt_test());
	// launch your tests here
	// TEST_OUTPUT("divide_error_test", divide_error_test());
	// TEST_OUTPUT("page_fault_zero_test", page_fault_zero_test());
	// TEST_OUTPUT("test_page_fault", test_page_fault());
	// TEST_OUTPUT("page_fault_big_test", page_fault_big_test());
	// TEST_OUTPUT("page_fault_null_test", page_fault_null_test());
	TEST_OUTPUT("boundrange_error_test", boundrange_error_test());
	
}
