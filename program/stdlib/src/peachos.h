#ifndef __PEACHOS_H__
#define __PEACHOS_H__

/**
 * @brief Print a string to the standard output.
 * @param str The null-terminated string to print.
 * @note This is implemented in assembly.
 */
void print(const char* str);

/**
 * @brief Get a character from the keyboard input.
 * @return The character read from the keyboard.
 * @note This is implemented in assembly.
 */
char getchar();

#endif // __PEACHOS_H__