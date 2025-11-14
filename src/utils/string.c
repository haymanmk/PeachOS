#include "string.h"

/**
 * @file string.c
 * @brief String manipulation functions implementation.
 */

 char* strncpy(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}

int strcmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(const unsigned char*)str1 - *(const unsigned char*)str2;
}

bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

uint8_t char_to_digit(char c) {
    if (is_digit(c)) {
        return (uint8_t)(c - '0');
    }
    return 0; // Return 0 for non-digit characters
}

uint32_t strlen(const char* str) {
    uint32_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

uint32_t strnlen(const char* str, size_t maxlen) {
    uint32_t len = 0;
    while (len < maxlen && str[len] != '\0') {
        len++;
    }
    return len;
}

char* strcpy(char* dest, const char* src) {
    char* original_dest = dest;
    while ((*dest++ = *src++) != '\0'); // Copy string including null terminator
    return original_dest;
}