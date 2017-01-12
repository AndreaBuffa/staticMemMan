#ifndef STATIC_MEM_MAN_C
#define STATIC_MEM_MAN_C
#include "stdio.h"
#include "staticMemManager.h"

#define GET_NUM_BLOCKS(numBytes) 32 * numBytes / (1028 * 32 + 1);
#define GET_NUM_WORDS(numBlocks) (numBlocks % 32 == 0) ? numBlocks / 32 : numBlocks / 32 + 1;
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

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
	for (uint32_t blockIt = 1; blockIt <= numBlocks; blockIt++) {
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
	uint32_t* pBitmap = (uint32_t*)handle + 1;
	uint32_t totBlocks = GET_NUM_BLOCKS(*pHandle);
	if (size == 0 || size > totBlocks) {
		return 3;
	}
	// if size is 3, sizeBitmap is 111
	uint32_t sizeBitmap = ((1 << size) - 1);
	// look for the first pattern corresponding to sizeBtimap
	uint32_t wordIt = 0, offset = 0, found = 0;
	uint32_t bitmapLen = GET_NUM_WORDS(totBlocks);
	while (wordIt < bitmapLen) {
		while (size + offset <= MIN(32, totBlocks)) {
			//printf("BITM %x\n", (*(pBitmap + wordIt)));
			//printf("PATT %x\n", sizeBitmap);
			if ((*(pBitmap + wordIt) & sizeBitmap) == sizeBitmap) {
				*(pBitmap + wordIt) ^= sizeBitmap;
				found = 1;
				break;
			}
			sizeBitmap = sizeBitmap << 1;
			offset++;
		}
		if (found)
			break;
		wordIt++;
	}
	printf("FOUND %d, word %d, offset %d\n", found, wordIt, offset);
	if (!found)
		return 5;

	// update the pointers area.
	uint32_t* pBlocks = pHandle + 1 + bitmapLen;
	uint32_t blockIt = 0;
	uint32_t blockIndex = wordIt * 32 + offset + 1; //1..n
	uint32_t* pNewChunk = pHandle + (*pHandle / 4) - \
		(blockIndex * 1024 / 4);

	while (blockIt < size) {
		*((uint32_t*)(pBlocks + wordIt * 32 + offset + blockIt)) = pNewChunk;
		blockIt++;
	}
	*((uint32_t*)ptr) = pNewChunk;
	return 0;
}

uint32_t memFree(void * const handle, const void* ptr) {
	uint32_t* pHandle = (uint32_t*)handle;
	uint32_t* pBitmap = pHandle + 1;
	uint32_t totBlocks = GET_NUM_BLOCKS((*pHandle));
	uint32_t bitmapSize = GET_NUM_WORDS(totBlocks);
	uint32_t* pBlocks = pHandle + 1 + bitmapSize;
	uint32_t idx = 0, found = 0, chunkStart = 0, chunkLen = 0;
	while (idx < totBlocks) {
		if ((*(pBlocks + idx)) == (*(uint32_t*)ptr)) {
			*(pBlocks + idx) = pHandle + (*pHandle / 4) - \
				((idx + 1) * 1024 / 4);
			if (!found) {
				chunkStart = idx;
			}
			found = 1;
			chunkLen++;
		} else if (found)
			break;
		idx++;
	}
	printf("FOUND checunk chunkLen %d, start %d\n", chunkLen, chunkStart);
	if (!found)
		return 4;
	uint32_t sizeBitmap = ((1 << chunkLen) - 1);
		printf("FOUND sizeBitmap %x\n", sizeBitmap);
	// get the word containing the bitmap to be updated
	*(pBitmap + (chunkStart / 32)) |= sizeBitmap << (chunkStart % 32);
	return 0;
}

uint32_t memDump(void* const handle) {
	uint32_t* pHandle = (uint32_t*)handle;
	uint32_t numBlocks = GET_NUM_BLOCKS((*pHandle));
	uint32_t numWords = GET_NUM_WORDS(numBlocks);
	printf("memDump Total buffer size: 0x%X\n", *pHandle);
	printf("memDump Total num of 1024B blocks: %d\n", numBlocks);
	printf("memDump Allocation Bitmap: ");
	for (uint32_t i = 0; i < numWords; i++) {
		printf(" 0x%X ", (uint32_t)*(pHandle + 1 + i));
	}
	printf("\n");
	printf("memDump Blocks pointers area: \n");
	uint32_t* pBlock = pHandle + 1 + numWords;
	for (uint32_t blockIdx = 0; blockIdx < numBlocks; blockIdx++) {
		printf(" 0x%X\n", (uint32_t)*(pBlock + blockIdx));
	}
	printf("\n");
	//printf("memDump Allocated blocks: %d\n", (32 * (*pHandle) / (1028 * 32 + 1)));
	return 0;
}

#endif // STATIC_MEM_MAN_C
