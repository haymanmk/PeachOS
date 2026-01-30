#include "stdlib.h"
#include "peachos.h"

/**
 * @brief Allocate memory of the specified size.
 * @param size The size of memory to allocate in bytes.
 * @return Pointer to the allocated memory, or NULL if allocation fails.
 */
void* malloc(size_t size) {
    return peachos_malloc(size);
}

void free(void* ptr) {
    peachos_free(ptr);
    return;
}