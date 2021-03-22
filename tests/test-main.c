#include "test-helper.h"

extern void test_http_state_machine(void);
extern void test_http_meth_two(void);
extern void test_zero_mem_alloc(void);
extern void test_SSE_zero_mem_alloc(void);

int main(int argc, const char *argv[]){
	test_http_state_machine();
	test_http_meth_two();
	test_zero_mem_alloc();
	test_SSE_zero_mem_alloc();
	
	return 0;
}
