#ifndef STATIC_MEM_MAN_C
#define STATIC_MEM_MAN_C
#include "stdio.h"
#include "staticMemManager.h"


uint32_t memInit(void * const handle, const uint32_t sizeInByte) {

	if (!handle)
		return 1;

	if  ((sizeInByte % 1024 != 0) || (sizeInByte <= 1024))
		return 2;

	uint32_t* pHandle = (uint32_t*)handle;
	*pHandle = sizeInByte;
	printf("SIZE 0x%X\n", *pHandle);
	// 1024x + 4x + x/32 = sizeInByte
	uint32_t numBlocks = 32 * sizeInByte / (1028 * 32 + 1);
	printf("num blocks %d\n", numBlocks);

	// the number of bits required for storing the status of a mem block
	uint32_t numWords = (numBlocks % 32 == 0) ? numBlocks / 32 : numBlocks / 32 + 1;

	uint32_t idx = 1;
	pHandle++;
	for (; idx <= numWords; idx++) {
		//@todo se e' l'ultima word...
		*(pHandle++) = 0xFFFFFFFF;
	}

	// foreach block set two words, the first for storing the chunck 
	// address and the second for storing the lenght.
	printf("HANDLE 0x%X size in bytes 0x%X\n", (uint32_t*)handle, sizeInByte);
	uint32_t* tmp = handle + sizeInByte / 4;
	printf("END 0x%X\n", tmp);
	for (uint32_t blockIt = 1; blockIt < numBlocks; blockIt++) {
		tmp = handle + sizeInByte / 4 - (blockIt * 1024 / 4);
		printf("Prima 0x%x\n", *((uint32_t*)(handle + idx + blockIt)));
		*((uint32_t**)(handle + idx + blockIt)) = tmp;
		printf("0x%x\n", tmp);
	}

	return 0;
}

uint32_t memAlloc(void * const handle, const uint32_t size, void * ptr);

uint32_t memFree(void * const handle, const void* ptr);

uint32_t memBulkFree(void * const handle);

uint32_t memDump(void* const handle) {
	uint32_t* pHandle = (uint32_t*)handle;
	printf("Total buffer size: 0x%X\n", *pHandle);
	printf("Total num of 1024B blocks: %d\n", (32 * (*pHandle) / (1028 * 32 + 1)));
	return 0;
}

#endif // STATIC_MEM_MAN_C
