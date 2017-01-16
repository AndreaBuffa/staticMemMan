#ifndef STATIC_MEM_MAN_C
#define STATIC_MEM_MAN_C
#include "stdio.h"
#include "staticMemManager.h"

#define BLK_SIZE 1024
//#define GET_NUM_BLOCKS(numBytes) 32 * numBytes / (1032 * 32 + 1);
#define GET_NUM_BLOCKS(numBytes) 64 * (numBytes - 8) / (1032 * 64 + 1);
#define GET_NUM_WORDS(numBlocks) (numBlocks % 64 == 0) ? numBlocks / 64 : numBlocks / 64 + 1;
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

uint32_t memInit(void * const handle, const uint32_t sizeInByte) {

	if (!handle)
		return 1;

	if  ((sizeInByte % 1024 != 0) || (sizeInByte <= BLK_SIZE))
		return 2;

	uint64_t* pHandle = (uint64_t*)handle;
	*pHandle = sizeInByte;
	// 1024x + 8x + x/64 + 8 = sizeInByte
	const uint32_t numBlocks = GET_NUM_BLOCKS(sizeInByte);
	// the number of bits required for storing the status of a mem block
	const uint32_t numWords = GET_NUM_WORDS(numBlocks);
	uint32_t idx = 0, remBits = numBlocks;

	do {
		idx++;
		if (idx == numWords) {
			*(++pHandle) = ((1 << remBits) - 1);
			*pHandle = (*pHandle == 0) ? 0xFFFFFFFFFFFFFFFF : *pHandle;
		} else {
			*(++pHandle) = 0xFFFFFFFFFFFFFFFF;
			remBits -= 64;
		}
	} while (idx < numWords);

	uint64_t* pBlocks = pHandle + 1;
	uint64_t* pFirstBlock = (uint64_t*)handle + \
		sizeInByte / 8 - (numBlocks * 1024 / 8);
	for (uint64_t blockIt = 0; blockIt < numBlocks; blockIt++) {
		*(pBlocks + blockIt) = (uint64_t)(pFirstBlock + (1024 * blockIt / 8));
		//printf("0x%x\n", (uint32_t)(pFirstBlock + (1024 * blockIt / 4)));
	}

	return 0;
}

uint32_t memAlloc(void * const handle, const uint32_t size, void * ptr) {
	if (!handle)
		return 1;
	uint64_t* pHandle = (uint64_t*)handle;
	uint64_t* pBitmap = (uint64_t*)handle + 1;
	uint32_t totBlocks = GET_NUM_BLOCKS(*pHandle);
	if (size == 0 || size > totBlocks) {
		return 3;
	}
	// if size is 3, sizeBitmap is 111
	uint64_t sizeBitmap = ((1 << size) - 1);
	// look for the first pattern corresponding to sizeBtimap
	uint32_t wordIt = 0, offset = 0, found = 0;
	uint32_t bitmapLen = GET_NUM_WORDS(totBlocks);
	while (wordIt < bitmapLen) {
		while (size + offset <= MIN(64, totBlocks)) {
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

	if (!found)
		return 5;
	//printf("FOUND %d, word %d, offset %d\n", found, wordIt, offset);

	// update the pointers area.
	uint64_t* pBlocks = pHandle + 1 + bitmapLen;
	uint32_t blockIt = 0;
	uint32_t blockIndex = wordIt * 64 + offset; //0..n-1
	uint64_t* pNew = pHandle + (*pHandle / 8) - \
		(totBlocks * 1024 / 8) + blockIndex * 1024 / 8;

	while (blockIt < size) {
		*((pBlocks + wordIt * 64 + offset + blockIt)) = (uint64_t)pNew ;
		blockIt++;
	}
	// return the pointer to the allocated chunk
	*((uint64_t**)ptr) = pNew;
	return 0;
}

uint32_t memFree(void * const handle, const void* ptr) {
	uint64_t* pHandle = (uint64_t*)handle;
	uint64_t* pBitmap = pHandle + 1;
	uint32_t totBlocks = GET_NUM_BLOCKS((*pHandle));
	uint32_t bitmapSize = GET_NUM_WORDS(totBlocks);
	uint64_t* pBlocks = (uint64_t*)(pHandle + 1 + bitmapSize);
	uint32_t idx = 0, found = 0, chunkStart = 0, chunkLen = 0;
	while (idx < totBlocks) {
		if ((*(pBlocks + idx)) == (*(uint64_t*)ptr)) {
			//printf("PT %x, idx %d\n", *(pBlocks + idx), idx);
			*(pBlocks + idx) = (uint64_t)(pHandle + (*pHandle / 8) - \
				(totBlocks * 1024 / 8) + (idx * 1024 / 8));
			if (!found) {
				chunkStart = idx;
			}
			found = 1;
			chunkLen++;
		} else if (found)
			break;
		idx++;
	}
	//printf("FOUND chunkLen %d, start %d\n", chunkLen, chunkStart);
	if (!found)
		return 4;
	uint64_t sizeBitmap = ((1 << chunkLen) - 1);
	//printf("FOUND sizeBitmap %x\n", sizeBitmap);
	// get the word containing the bitmap to be updated
	*(pBitmap + (chunkStart / 64)) |= sizeBitmap << (chunkStart % 64);
	return 0;
}

uint32_t memDump(void* const handle, uint32_t verb) {
	uint64_t* pHandle = (uint64_t*)handle;
	uint32_t numBlocks = GET_NUM_BLOCKS((*pHandle));
	uint32_t numWords = GET_NUM_WORDS(numBlocks);
	printf("memDump Total buffer size: 0x%lX\n", *pHandle);
	printf("memDump Total num of 1024B blocks: %d\n", numBlocks);
	printf("memDump Num words for the alloc bitmap %d\n", numWords);

	if (verb == 1) {

		printf("memDump Allocation Bitmap: ");
		for (uint32_t i = 0; i < numWords; i++) {
			printf(" 0x%lX ", *(pHandle + 1 + i));
		}
		printf("\n");

		printf("memDump Blocks pointers area: \n");
		uint64_t* pBlock = pHandle + 1 + numWords;
		for (uint32_t blockIdx = 0; blockIdx < numBlocks; blockIdx++) {
			printf("memDump block %4d pointer 0x%lX\n", blockIdx + 1,
				*(pBlock + blockIdx));
		}
		printf("\n");
	}
	//printf("memDump Allocated blocks: %d\n", (32 * (*pHandle) / (1028 * 32 + 1)));
	return 0;
}

#endif // STATIC_MEM_MAN_C
