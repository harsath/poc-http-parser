#include "test-helper.h"

extern void test_http_state_machine(void);
extern void test_http_meth_two(void);

int main(int argc, const char *argv[]){
	test_http_state_machine();
	test_http_meth_two();
	
	return 0;
}
