#ifndef STATIC_MEM_MAN_C
#define STATIC_MEM_MAN_C
#include "stdio.h"
#include "staticMemManager.h"

#define GET_NUM_BLOCKS(numBytes) 32 * numBytes / (1028 * 32 + 1);
#define GET_NUM_WORDS(numBlocks) (numBlocks % 32 == 0) ? numBlocks / 32 : numBlocks / 32 + 1;

markAndSet(uint32_t* pHandle, uint32_t* pChunck, uint32_t op) {


}

uint32_t memInit(void * const handle, const uint32_t sizeInByte) {

	if (!handle)
		return 1;

	if  ((sizeInByte % 1024 != 0) || (sizeInByte <= 1024))
		return 2;

	uint32_t* pHandle = (uint32_t*)handle;
	*pHandle = sizeInByte;
	printf("SIZE 0x%X\n", *pHandle);
	// 1024x + 4x + x/32 = sizeInByte
	uint32_t numBlocks = GET_NUM_BLOCKS(sizeInByte);
	printf("num blocks %d\n", numBlocks);

	// the number of bits required for storing the status of a mem block
	const uint32_t numWords = (numBlocks % 32 == 0) ? numBlocks / 32 : \
		numBlocks / 32 + 1;
	printf("num words for storing the status bitmap %d\n", numWords);
	uint32_t idx = 0, remBits = numBlocks;

	do {
		idx++;
		if (idx == numWords) {
			*(++pHandle) = ((1 << remBits) - 1);
		} else {
			*(++pHandle) = 0xFFFFFFFF;
			remBits -= 32;
		}
	} while (idx < numWords);

	//printf("%d\n", idx);
	// foreach block set two words, the first for storing the chunck 
	// address and the second for storing the lenght.
	printf("HANDLE 0x%X size in bytes 0x%X\n", (uint32_t)handle, sizeInByte);
	for (uint32_t blockIt = 1; blockIt < numBlocks; blockIt++) {
		const uint32_t* tmp = ((uint32_t*)handle) + sizeInByte / 4 - \
			(blockIt * 1024 / 4);
		*((uint32_t**)(pHandle + blockIt)) = tmp;
		printf("0x%x\n", (uint32_t)tmp);
	}

	return 0;
}

uint32_t memAlloc(void * const handle, const uint32_t size, void * ptr) {
	if (!handle)
		return 1;
	uint32_t* pHandle = (uint32_t*)handle;
	uint32_t numBlocks = GET_NUM_BLOCKS(*pHandle);
	if (size == 0 || size > numBlocks) {
		return 3;
	}
	markAndSet(pHandle, (uint32_t*)ptr, 0);
	return 0;
}

uint32_t memFree(void * const handle, const void* ptr) {
	uint32_t* pHandle = (uint32_t*)handle;
	uint32_t numBlocks = GET_NUM_BLOCKS((*pHandle));
	uint32_t numWords = GET_NUM_WORDS(numBlocks);
	uint32_t idx = 0;
	while (idx < numBlocks) {
		if ((uint32_t*)*(pHandle + 1 + numWords + idx) == (uint32_t*)ptr) {
			*(pHandle + 1 + numWords + idx) = 0;
			//1 << idx
			//pHandle + 1 + (idx / 32)
		}
		idx++;
	}
}

uint32_t memDump(void* const handle) {
	uint32_t* pHandle = (uint32_t*)handle;
	uint32_t numBlocks = GET_NUM_BLOCKS((*pHandle));
	uint32_t numWords = GET_NUM_WORDS(numBlocks);
	printf("memDump Total buffer size: 0x%X\n", *pHandle);
	printf("memDump Total num of 1024B blocks: %d\n", numBlocks);
	printf("memDump Allocation Bitmap: ");
	for (uint32_t i = 0; i < numWords; i++) {
		printf(" %X ", (uint32_t)*(++pHandle));
	}
	printf("\n");


	printf("memDump Allocated blocks: %d\n", (32 * (*pHandle) / (1028 * 32 + 1)));
	return 0;
}

#endif // STATIC_MEM_MAN_C
