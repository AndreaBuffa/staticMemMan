# staticMemMan

* Simple static memory manager.
*
* Use a buffer and split it in a certain number of blocks (i.e. 1024B large).
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
* For the example above, the buffer can hold up to 9 1024B blocks.
*
* The first 64bits word is the size of the buffer. The following words
* are used for the allocation bitmap (1 == free, 0 == used). In this case
* one word is enought because we have a total number of blocks equal to
* 9. Then, 9 words are used to store the pointers to the corresponding
* blocks inside the buffer.
*
*
* 
* lenght | allocation | blocks pointers list |memory used for allocating
		 |   bitmap   |                              the chunks
* -------------------------------------------------...------------------
* |0x2800|000000001111|  pBlock1..pBlok9     |                         |
* -------------------------------------------------...------------------
* (64bits)                ( 9 * 64bits)		 
*     							 				 ^               ^
*      			               |_________|_______|               |
*                                        |_______________________|
*
