#ifndef STATIC_MEM_MAN_H
#define STATIC_MEM_MAN_H

#include "stdint.h"

/**
* Simple static memory manager.
*
* Take a buffer and split it in a certain number of blocks, 1024B large.
*
* About 1% of the buffer is used for storing the status of the
* allocations.
*
* Example usage.
*
* char buffer[10240];
* memInit((void*)buffer, size);
* uint32_t* p1 = 0;
* memAlloc((void*)buffer, 4, &p1);
* //p1 is now a pointer to a memory 4 * 1024Bs
* memFree((void*)buffer, &p1);
*
* Allocation scheme.
* For the example above, the buffer can hold at most 9 1024B blocks.
*
* The first 32bits word is the size of the buffer. The following words
* are used for the allocation bitmap (1 == free, 0 == used). In this case
* one word is enought beacause we have a total number of blocks equal to
* 9. Then, 9 words are use to store the pointers to the corresponding
* blocks inside the buffer
*
*
* 
* lenght | blocks pointers list |
* -------------------------------------------------...------------------
* |0x2800|   pBlock1..pBlok9    |                                      |
* -------------------------------------------------...------------------
* (64bits)    ( 9 * 64bits)		 (memory used for allocating the chunks)
* 								 ^                                ^
* 			   |_________|_______|                                |
*                        |________________________________________|
*
*/
uint32_t memInit(void* const handle, const uint32_t sizeInByte);

uint32_t memAlloc(void* const handle, const uint32_t size, void* ptr);

uint32_t memFree(void* const handle, const void* ptr);

uint32_t memDump(void* const handle, uint32_t verb);

#endif // STATIC_MEM_MAN_H
