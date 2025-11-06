#include "memory.h"

/**
 * @brief Sets the first num bytes of the block of memory pointed by ptr to the specified value.
 * @param ptr Pointer to the block of memory to fill.
 * @param value Value to be set.
 * @param num Number of bytes to be set to the value.
 * @return A pointer to the memory area ptr.
 */
void *memset(void *ptr, uint8_t value, size_t num) {
    uint8_t *byte_ptr = (uint8_t *)ptr;
    for (size_t i = 0; i < num; i++) {
        byte_ptr[i] = value;
    }
    return ptr;
}
