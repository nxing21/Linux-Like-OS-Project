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
int idt_test(){
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

int divide_error_test(){
	TEST_HEADER;
	int a;
	int b;
	b = 0;
	a = 2 / b;
	return FAIL;
}


// add more tests here
int page_fault_zero_test(){
	TEST_HEADER;
	
	const int *bad_ptr = (const int *)0;
	const int lol = &bad_ptr;

	return FAIL;
}


int page_test(){
	TEST_HEADER;
	
	const int *good_ptr = 1024;
	int lol = &good_ptr;

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
	TEST_OUTPUT("sys_call_test", sys_call_test());
}
