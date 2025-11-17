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

int strcmp_ignore_case(const char* str1, const char* str2) {
    while (*str1 && (tolower(*str1) == tolower(*str2))) {
        str1++;
        str2++;
    }
    return (unsigned char)tolower(*str1) - (unsigned char)tolower(*str2);
}

int strncmp(const char* str1, const char* str2, size_t n) {
    size_t i;
    for (i = 0; i < n; i++) {
        if (str1[i] != str2[i] || str1[i] == '\0' || str2[i] == '\0') {
            return (unsigned char)str1[i] - (unsigned char)str2[i];
        }
    }
    return 0;
}

char tolower(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c + ('a' - 'A');
    }
    return c;
}

int strncmp_ignore_case(const char* str1, const char* str2, size_t n) {
    size_t i;
    for (i = 0; i < n; i++) {
        char c1 = tolower(str1[i]);
        char c2 = tolower(str2[i]);
        if (c1 != c2 || c1 == '\0' || c2 == '\0') {
            return (unsigned char)c1 - (unsigned char)c2;
        }
    }
    return 0;
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