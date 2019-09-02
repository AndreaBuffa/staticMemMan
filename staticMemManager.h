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
* The first 64bits word is the size of the buffer. The following words
* are used for the allocation bitmap (1 == free, 0 == used). In this case
* one word is enought because we have a total number of blocks equal to
* 9. Then, 9 words are used to store the pointers to the corresponding
* blocks inside the buffer.
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

/**
 * Initialize the backing buffer with the allocation bitmap.
 * @param handle, const pointer to the backing buffer
 * @param sizeInByte, the backing buffer size.
 * @return 1 if handle is null, 2 if the bancking buffer is smaller than
 * the chunk size, or if its size is not multiple of the chunk size.
 * 0 if success.
 */
uint32_t memInit(void* const handle, const uint32_t sizeInByte);

/**
 * Alloc a certain number size of chunks. A pointer to that is returned
 * @param handle, const pointer to the backing buffer.
 * @param size, the number of chunks to be allocated.
 * @param size, the pointer to the allocated area.
 * @return 1 if handle is null, 3 if either size (requested num of chunks)
 * is 0, or the size is bigger than the max num of possible allocation.
 * 0 if success.
 */
uint32_t memAlloc(void* const handle, const uint32_t size, void* ptr);

/**
 * @param handle, const pointer to the backing buffer.
 * @return 4, if ptr is not managed by this manager. 0 otherwise.
 */
uint32_t memFree(void* const handle, const void* ptr);

/**
 * @param handle, const pointer to the backing buffer.
 * @param verb, the verbosity
 * @return 0
 */
uint32_t memDump(void* const handle, uint32_t verb);

#endif // STATIC_MEM_MAN_H
