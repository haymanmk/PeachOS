#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stddef.h>
#include <stdint.h>

void *memset(void *ptr, uint8_t value, size_t num);
void *memcpy(void *dest, const void *src, size_t num);
int memcmp(const void *ptr1, const void *ptr2, size_t num);

#endif // __MEMORY_H__
