#include "peachos.h"
#include "stdio.h"
#include "stdlib.h"
#include <stdarg.h>
#include <stdint.h>

/**
 * @brief Put a character to the standard output.
 * @param c The character to print.
 * @return The character printed.
 */
int putchar(int c) {
    peachos_putchar((char)c);
    return c;
}

int printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    const char* ptr = format;
    uint32_t i;

    while (*ptr) {
        if (*ptr != '%') {
            putchar(*ptr++);
            i++;
            continue;
        }

        switch (*++ptr) {
            case 'c': {
                char c = (char)va_arg(args, int);
                putchar(c);
                i++;
                break;
            }
            case 's': {
                char* str = va_arg(args, char*);
                int res = peachos_print(str);
                if (res < 0) {
                    i = res;
                    goto exit; // Return error code
                }
                i += res;
                break;
            }
            case 'd':
            case 'i': {
                int value = va_arg(args, int);
                char* str = itoa(value);
                int res = peachos_print(str);
                if (res < 0) {
                    i = res;
                    goto exit; // Return error code
                }
                i += res;
                break;
            }
            default:
                putchar('%');
                putchar(*ptr);
                i += 2;
                break;
        }
        ptr++;
    }

exit:
    va_end(args);
    return i;
}