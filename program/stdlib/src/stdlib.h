#ifndef __STDLIB_H__
#define __STDLIB_H__

#include <stddef.h>

void* malloc(size_t size);
void free(void* ptr);
char* itoa(int value);

#endif // __STDLIB_H__