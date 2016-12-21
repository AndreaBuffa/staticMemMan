#ifndef STATIC_MEM_MAN_H
#define STATIC_MEM_MAN_H

#include "stdint.h"

uint32_t memInit(void* const handle, const uint32_t sizeInByte);

uint32_t memAlloc(void* const handle, const uint32_t size, void* ptr);

uint32_t memFree(void* const handle, const void* ptr);

uint32_t memDump(void* const handle);

#endif // STATIC_MEM_MAN_H
