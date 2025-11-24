#ifndef __STRING_H__
#define __STRING_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Convert a character to lowercase.
 * @param c The character to convert.
 * @return The lowercase character.
 */
char tolower(char c);

/**
 * @brief Convert a character to uppercase.
 * @param c The character to convert.
 * @return The uppercase character.
 */
char toupper(char c);

/**
 * @brief Copy a string from src to dest.
 * @param dest The destination buffer.
 * @param src The source string.
 * @param n The maximum number of bytes to copy.
 * @return The pointer to the destination buffer.
 */
char* strncpy(char* dest, const char* src, size_t n);

/**
 * @brief Compare two strings until the first null terminator or difference.
 * @param str1 The first string.
 * @param str2 The second string.
 * @return 0 if equal, negative if str1 < str2, positive if str1 > str2.
 */
int strcmp(const char* str1, const char* str2);

int strcmp_ignore_case(const char* str1, const char* str2);

/**
 * @brief Compare two strings up to n characters.
 * @param str1 The first string.
 * @param str2 The second string.
 * @param n The maximum number of characters to compare.
 * @return 0 if equal, negative if str1 < str2, positive if str1 > str2.
 */
int strncmp(const char* str1, const char* str2, size_t n);

/**
 * @brief Compare two strings up to n characters, ignoring case.
 * @param str1 The first string.
 * @param str2 The second string.
 * @param n The maximum number of characters to compare.
 * @return 0 if equal, negative if str1 < str2, positive if str1 > str2.
 */
int strncmp_ignore_case(const char* str1, const char* str2, size_t n);

/**
 * @brief Check if a character is a digit.
 * @param c The character to check.
 * @return true if the character is a digit, false otherwise.
 */
bool is_digit(char c);

/**
 * @brief Convert a character digit to its integer value.
 * @param c The character digit.
 * @return The integer value of the digit, or 0 if not a digit.
 */
uint8_t char_to_digit(char c);

/**
 * @brief Get the length of a string which is null-terminated.
 * @param str The string to measure.
 * @return The length of the string.
 */
uint32_t strlen(const char* str);

/**
 * @brief Get the length of a string up to a maximum length.
 * @param str The string to measure.
 * @param maxlen The maximum length to check.
 * @return The length of the string up to maxlen.
 */
uint32_t strnlen(const char* str, size_t maxlen);

/**
 * @brief Copy a string from src to dest.
 * @param dest The destination buffer.
 * @param src The source string.
 * @return The pointer to the destination buffer.
 */
char* strcpy(char* dest, const char* src);

#endif // __STRING_H__