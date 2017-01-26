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
			*(++pHandle) = (remBits == 64) ? 0xFFFFFFFFFFFFFFFF : \
				((1L << remBits) - 1);
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

int32_t searchBitmap(uint64_t * const pBitmap, uint32_t steps, 
					 const uint32_t size, const uint8_t strict) {
	if (steps == 0)
		return -1;
	steps--;

	uint64_t sizeBitmap = 0xFFFFFFFFFFFFFFFF;
	// if size is 3, sizeBitmap is 111
	sizeBitmap = (size < 64) ? ((1L << size) - 1) : sizeBitmap;
	printf("CURR   BITM %lx\n", *pBitmap);
	printf("SEARCH PATT %16lx\n", sizeBitmap);
	if ((*pBitmap & sizeBitmap) == sizeBitmap) {
		if (size <= 64)
			return 0;
		else 
			return searchBitmap(pBitmap + 1, steps, size - 64, 1);
	} else {
		if (strict == 1) {
			return -1;
		}
		// 
		uint32_t offset = 1;
		sizeBitmap = sizeBitmap << 1;

		while (offset < 64) {
			printf("SEARCH PATT %16lx\n", sizeBitmap);
			if ((*pBitmap & sizeBitmap) == sizeBitmap) {
				printf("FOUND size %d, offset %d\n", size, offset);
				int remainder = size - 64 - offset;
				if (remainder > 0) {
					int32_t ret = searchBitmap(pBitmap + 1, steps, remainder, 1);
					return ((ret == 0) ? offset : -1);
				}
				return offset;
			}
			sizeBitmap = sizeBitmap << 1;
			offset++;
		}
		if (offset == 64) {
			return 64 + searchBitmap(pBitmap + 1, steps, size, 0);
		}
	}
	return -1;
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
	uint32_t numWords = GET_NUM_WORDS(totBlocks);
	uint32_t wordIt = 0, offset = 0, found = 0;
	uint32_t startIndex = 0;
	if ((startIndex = searchBitmap(pBitmap, numWords, size, 0)) == -1)
		return 5;

	// update allocation bitmap
	uint64_t sizeBitmap = 0xFFFFFFFFFFFFFFFF;
	wordIt = startIndex / 64;
	offset = startIndex % 64;
	printf("FOUND start %d, word %d, offset %d\n", startIndex, wordIt, offset);
	uint32_t remainder = size;
	while (remainder > 0) {
		if (startIndex > (wordIt + 1) * 64) {
			// the first word
			sizeBitmap = 0xFFFFFFFFFFFFFFFF << offset;
			//printf("PATT %lx\n", sizeBitmap);
			*(pBitmap + wordIt++) ^= sizeBitmap;
			remainder -= offset;
		} else {
			if (remainder >= 64) {
				remainder -= MIN(64, size);
				*(pBitmap + wordIt++) ^= sizeBitmap;
			} else {
				sizeBitmap = (1L << remainder) - 1;
				sizeBitmap = sizeBitmap << offset;
				*(pBitmap + wordIt++) ^= sizeBitmap;
				//printf("TEST %lx\n", sizeBitmap);
				remainder = 0;
			}
		}
		printf("WRITE %lx\n", sizeBitmap);
	}

	// update the pointers area.
	uint64_t* pBlocks = pHandle + 1 + numWords;
	uint32_t blockIt = 0;
	uint64_t* pNew = pHandle + (*pHandle / 8) - \
		(totBlocks * 1024 / 8) + startIndex * 1024 / 8;

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
	printf("memDump Total buffer size: 0x%lX (%ld)\n", *pHandle, *pHandle);
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
