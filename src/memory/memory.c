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

/**
 * @brief Copies num bytes from memory area src to memory area dest.
 * @param dest Pointer to the destination memory area.
 * @param src Pointer to the source memory area.
 * @param num Number of bytes to copy.
 * @return A pointer to the destination memory area dest.
 */
void* memcpy(void *dest, const void *src, size_t num) {
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    for (size_t i = 0; i < num; i++) {
        d[i] = s[i];
    }
    return dest;
}

/**
 * @brief Compares the first num bytes of two memory areas.
 * @param ptr1 Pointer to the first memory area.
 * @param ptr2 Pointer to the second memory area.
 * @param num Number of bytes to compare.
 * @return An integer less than, equal to, or greater than zero if the first num bytes of ptr1
 *         is found, respectively, to be less than, to match, or be greater than the first num bytes of ptr2.
 */
int memcmp(const void *ptr1, const void *ptr2, size_t num) {
    const uint8_t *p1 = (const uint8_t *)ptr1;
    const uint8_t *p2 = (const uint8_t *)ptr2;
    for (size_t i = 0; i < num; i++) {
        if (p1[i] != p2[i]) {
            return (int)(p1[i] - p2[i]);
        }
    }
    return 0;
}