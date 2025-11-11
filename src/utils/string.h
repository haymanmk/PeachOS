#ifndef __STRING_H__
#define __STRING_H__

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Copy a string from src to dest.
 * @param dest The destination buffer.
 * @param src The source string.
 * @param n The maximum number of bytes to copy.
 * @return The pointer to the destination buffer.
 */
char* strncpy(char* dest, const char* src, size_t n);

/**
 * @brief Compare two strings.
 * @param str1 The first string.
 * @param str2 The second string.
 * @return 0 if equal, negative if str1 < str2, positive if str1 > str2.
 */
int strcmp(const char* str1, const char* str2);

#endif // __STRING_H__