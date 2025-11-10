#ifndef __KHEAP_H__
#define __KHEAP_H__

#include <stddef.h>
#include "config.h"

// Kernel heap management functions
void kheap_init();
void* kheap_malloc(size_t size);
void* kheap_zmalloc(size_t size);
void kheap_free(void* ptr);

#endif // __KHEAP_H__
