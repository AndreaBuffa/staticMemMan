#include "stdio.h"
#include "staticMemManager.h"

#define BUFFER_SIZE 10240
char buffer[BUFFER_SIZE];

int main(void) {
	uint32_t line = 0;
	printf("Buffer 0x%x\n", ((uint32_t*)buffer));
	memInit((void*)buffer, BUFFER_SIZE);

	memDump((void*)buffer);

	//test 
	uint32_t* tmp = buffer;
	printf("Size is %d\n", ((uint32_t)*tmp));
	if (*tmp != BUFFER_SIZE) {
		line = __LINE__;
		goto error;
	}

	return 0;

error:
	printf("Test at line %d FAILED\n", line);
	return 0;
}
