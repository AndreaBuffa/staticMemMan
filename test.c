#include "stdio.h"
#include "staticMemManager.h"

#define BUFFER_SIZE_1 10240
//#define BUFFER_SIZE_2 0x2000000
#define BUFFER_SIZE_2 0x10400
char smallBuffer[BUFFER_SIZE_1];
char bigBuffer[BUFFER_SIZE_2];


int test(void* pBuf, uint32_t size) {
	uint32_t line = 0;
	uint32_t p1 = 0;
	uint32_t* p2 = 0;
	uint32_t* p3 = 0;
	printf("Buffer pointer 0x%X\n", pBuf);
	memInit((void*)pBuf, size);

	memDump((void*)pBuf, 1);

	//Check the first word is equal to the lenght BUFFER_SIZE
	uint32_t* tmp = (uint32_t*)pBuf;
	printf("Test Size: Size is %d\n", ((uint32_t)*tmp));
	if (*tmp != size) {
		line = __LINE__;
		goto error;
	}

	// check the the allocation bitmap

	// check mem allocation
	// request more blocks than available
	printf("Test memAlloc oversize\n");
	if (0 == memAlloc((void*)pBuf, size * 2, &p1)) {
		line = __LINE__;
		goto error;
	}

	printf("Test memAlloc 1 block\n");
	if (0 != memAlloc((void*)pBuf, 1, &p1)) {
		line = __LINE__;
		goto error;
	}

	//fill p1
	for(uint32_t i = 0; i < (1 * 1024 / 4); i++) {
		*(((uint32_t*)p1) + i) = i;
	}

	printf("Test memAlloc oversize 2\n");
	//must fail
	if (0 == memAlloc((void*)pBuf, 1 + size, &p2)) {
		line = __LINE__;
		goto error;
	}

	printf("Test memAlloc 4 blocks\n");
	if (0 != memAlloc((void*)pBuf, 4, &p2)) {
		line = __LINE__;
		goto error;
	}

	// fill p2
	for(uint32_t i = 0; i < (4 * 1024 / 4); i++) {
		*(p2 + i) = i;
		//printf("0x%X, val %d \n", p2 + i, *(p2 + i));

	}

	// check p1 data
	for(uint32_t i = 0; i < (1 * 1024 / 4); i++) {
		if (*((uint32_t*)p1 + i) != i) {
			printf("FAIL idx %d, val %d \n", i, *((uint32_t*)p1 + i));
			line = __LINE__;
			goto error;
		}
	}

	printf("Test memAlloc other 4 blocks 5\n");
	if (0 != memAlloc((void*)pBuf, 4, &p3)) {
		line = __LINE__;
		goto error;
	}

	// fill p3
	for(uint32_t i = 0; i < (4 * 1024 / 4); i++) {
		*(p3 + i) = i;
		//printf("%d ", *(p3 + i));
	}

	// check p2 data
	for(uint32_t i = 0; i < (4 * 1024 / 4); i++) {
		if (*(p2 + i) != i) {
			printf("FAIL idx %d, val %d", i, *(p2 + i));
			line = __LINE__;
			goto error;
		}
	}

	printf("Test MemFree p3. Deleting 0x%X\n", p3);
	if (0 != memFree((void*)pBuf, &p3)) {
		line = __LINE__;
		goto error;
	}

	printf("Test MemFree p1. Deleting 0x%X\n", p1);
	if (0 != memFree((void*)pBuf, &p1)) {
		line = __LINE__;
		goto error;
	}

	printf("Test MemFree p2. Deleting 0x%X\n", p2);
	if (0 != memFree((void*)pBuf, &p2)) {
		line = __LINE__;
		goto error;
	}

	//memDump((void*)pBuf, 1);
	printf("SUCCESS!!!!!!!!\n");
	return 0;

error:
	memDump((void*)pBuf, 1);
	printf("Test at line %d FAILED\n", line);
	return 1;
}

test2(void* pBuf, uint32_t size) {
	uint32_t* p1 = 0;
	uint32_t* p2 = 0;
	uint32_t* p3 = 0;
	uint32_t* p4 = 0;
	uint32_t* p5 = 0;
	uint32_t line = 0;

	memInit((void*)pBuf, size);

	printf("Test memAlloc 40 blocks 1\n");
	if (0 != memAlloc((void*)pBuf, 40, &p1)) {
		line = __LINE__;
		goto error;
	}

	printf("Test memAlloc 40 blocks 2\n");
	if (0 != memAlloc((void*)pBuf, 40, &p2)) {
		line = __LINE__;
		goto error;
	}

	memDump((void*)pBuf, 1);

	return 0;
error:
	memDump((void*)pBuf, 0);
	printf("Test at line %d FAILED\n", line);
	return 1;
}

int main(void) {
	if (0 != test(smallBuffer, BUFFER_SIZE_1)) {
		goto error;
	}

	printf("Use another buffer *********\n");
	if (0 != test(bigBuffer, BUFFER_SIZE_2)) {
		goto error;
	}
	printf("More extra test    *********\n");
	if (0 != test2(bigBuffer, BUFFER_SIZE_2)) {
		goto error;
	}

	printf("SUCCESS!!!!!!!!\n");
	return 0;
error:
	return 1;
}
