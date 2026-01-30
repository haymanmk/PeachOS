#include "stdlib.h"
#include "peachos.h"
#include <stdint.h>

#define MAX_DIGITS 12 // Maximum digits for 32-bit integer including sign and null terminator

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

char* itoa(int value) {
    static char buffer[MAX_DIGITS]; // Buffer to hold the string representation
    char* ptr = &buffer[MAX_DIGITS - 1];
    *ptr = '\0';
    uint8_t is_negative = 0;
    if (value < 0) {
        is_negative = 1;
        value = -value;
    }

    while (value != 0) {
        int rem = value % 10;
        *(--ptr) = (char)(rem + '0');
        value /= 10;
    }
    if (is_negative) {
        *(--ptr) = '-';
    }
    if (*ptr == '\0') { // Handle the case when value is 0
        *(--ptr) = '0';
    }
    return ptr;
}