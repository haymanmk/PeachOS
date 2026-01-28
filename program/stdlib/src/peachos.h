#ifndef __PEACHOS_H__
#define __PEACHOS_H__

#include <stddef.h>

/**
 * @brief Print a string to the standard output.
 * @param str The null-terminated string to print.
 * @note This is implemented in assembly.
 */
void peachos_print(const char* str);

/**
 * @brief Get a character from the keyboard input.
 * @return The character read from the keyboard.
 * @note This is implemented in assembly.
 */
char peachos_getchar();

/**
 * @brief Allocate memory of the specified size.
 * @param size The size of memory to allocate in bytes.
 * @return Pointer to the allocated memory, or NULL if allocation fails.
 * @note This function is implemented in assembly.
 */
void* peachos_malloc(size_t size);

#endif // __PEACHOS_H__