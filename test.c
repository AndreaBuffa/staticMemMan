#include "stdio.h"
#include "staticMemManager.h"

#define BUFFER_SIZE 10240
char buffer[BUFFER_SIZE];

int checkAllocationBitmap(void);


int main(void) {
	uint32_t line = 0;
	uint32_t p1, p2, p3 = 0;
	printf("Buffer pointer 0x%x\n", ((uint32_t)buffer));
	memInit((void*)buffer, BUFFER_SIZE);

	memDump((void*)buffer);

	//Check the first word is equal to the lenght BUFFER_SIZE
	uint32_t* tmp = (uint32_t*)buffer;
	printf("Test Size: Size is %d\n", ((uint32_t)*tmp));
	if (*tmp != BUFFER_SIZE) {
		line = __LINE__;
		goto error;
	}

	// check the the allocation bitmap

	// check mem allocation
	// request more blocks than available
	printf("Test Allocations 1\n");
	if (0 == memAlloc((void*)buffer, 20, &p1)) {
		line = __LINE__;
		goto error;
	}
	memDump((void*)buffer);

	printf("Test Allocations 2\n");
	if (0 != memAlloc((void*)buffer, 1, &p1)) {
		line = __LINE__;
		goto error;
	}
	memDump((void*)buffer);	

	printf("Test Allocations 3\n");
	//must fail
	if (0 == memAlloc((void*)buffer, 9, &p2)) {
		line = __LINE__;
		goto error;
	}

	printf("Test Allocations 4\n");
	if (0 != memAlloc((void*)buffer, 4, &p2)) {
		line = __LINE__;
		goto error;
	}
	memDump((void*)buffer);

	printf("Test Allocations 5\n");
	if (0 != memAlloc((void*)buffer, 4, &p3)) {
		line = __LINE__;
		goto error;
	}	
	memDump((void*)buffer);

	printf("Test MemFree 1. Deleting 0x%X\n", p3);
	if (0 != memFree((void*)buffer, &p3)) {
		line = __LINE__;
		goto error;
	}
	memDump((void*)buffer);

	printf("Test MemFree 2. Deleting 0x%X\n", p1);
	if (0 != memFree((void*)buffer, &p1)) {
		line = __LINE__;
		goto error;
	}
	memDump((void*)buffer);
	printf("SUCCESS!!!!!!!!\n");
	return 0;

error:
	printf("Test at line %d FAILED\n", line);
	return 0;
}
