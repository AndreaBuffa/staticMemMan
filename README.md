# staticMemMan

Simple static memory manager.

Manage a static allocated buffer as a set of fixed-size memory chunks,
giving the possibility to alloc and free a certain number of them.
Allocated chunks are contiguous.

Example usage.

char buffer[10240];
memInit((void*)buffer, size);
uint32_t* p1 = 0;
memAlloc((void*)buffer, 4, &p1);
//p1 is now a pointer to a memory 4 * 1024Bs
	memFree((void*)buffer, &p1);

Allocation scheme.


For the example above, the given buffer can hold up to 9 chunks
(each 1024B large).
Internally, a certain number of words are used for tracking information
about wich chunks is free or not, the list of pointers to each chunk, 
and the length of each chunk.

The first 64bits word is the size of the buffer. Then, there is the
allocation bitmap (1 == free, 0 == used). Then there is the chuncks 
pointed list and finally the area reserved for each chunk.
In the above example, a one-word allocation bitmap is used, since the
total number of alloc-able chunks is 9 and one word is
enought to store 9 bits.
The chunks pointer list is made of 9 words. The rest of the words are
used for the chunks.


  lenght | allocation | chunks pointers list |memory used for allocating
		 |   bitmap   |                              the chunks
 -------------------------------------------------...------------------
 |0x2800 |000000001111|  pBlock1..pBlok9     |                         |
 -------------------------------------------------...------------------
 (64bits)                ( 9 * 64bits)		 
     							 				 ^               ^
      			               |_________|_______|               |
                                         |_______________________|
                                         

Given a backing buffer, sizeInByte big, the number of chunks of 1024B, 
(considering a 64 bits architecture) is:

1024x + 8x + x/64 + 8 = sizeInByte  (x/64 as ceiling(x/64) )

1032x + x/64 = sizeInByte - 8

1032*64x + x = 64 * (sizeInByte - 8)

x = 64 * (sizeInByte - 8) / (1032 * 64 + 1);

Hence, given a buffer of 10240 bytes,

sizeInByte = 10240
1024x + 8x + x/64 + 8 = 10240
x = 64 * (10240 - 8) / (1032 * 64 + 1);
x = 9

that is, 9 chunks of 1024 bytes can be allocated.

Roughly, about 10% of space is wasted for the chunks management.

In terms of time, the time for finding m chunks space, out of n possible,
 is the time to visit the allocation bitmap. 
 
 m * n comparison must be done hence O(n^2).






