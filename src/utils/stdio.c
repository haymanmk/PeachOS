#include "stdio.h"
#include "string.h"
#include <stdarg.h>
#include "io/io.h"

uint16_t *video_memory = (uint16_t *)0xB8000;
uint16_t vx = 0;
uint16_t vy = 0;

void disable_cursor() {
    outsb(0x3D4, 0x0A);
    outsb(0x3D5, 0x20);
}

uint16_t create_char(char c, uint8_t fg, uint8_t bg) {
    return (bg << 12) | (fg << 8) | c;
}

void put_char(int x, int y, char c, uint8_t fg, uint8_t bg) {
    video_memory[y * VIDEO_WIDTH + x] = create_char(c, fg, bg);
}

void print_char(char c, uint8_t fg, uint8_t bg) {
    if (vx < 0 || vx >= VIDEO_WIDTH || vy < 0 || vy >= VIDEO_HEIGHT) {
        return; // Out of bounds
    }

    // detect carriage return and new line feed
    if (c == '\n') {
        vx = 0;
        vy++;
        if (vy >= VIDEO_HEIGHT) {
            return; // No more space
        }
        return;
    }
    if (c == '\r') {
        vx = 0;
        return;
    }

    put_char(vx, vy, c, fg, bg);

    // update values of x and y
    vx++;
    if (vx >= VIDEO_WIDTH) {
        vx = 0;
        if (++vy >= VIDEO_HEIGHT) {
            return; // No more space
        }
    }
}

static void print_string(const char* str, uint8_t fg, uint8_t bg) {
    while (*str) {
        print_char(*str++, fg, bg);
    }
}

static void itoa(int num, char* buf, int base) {
    char* ptr = buf, *ptr1 = buf, tmp_char;
    long long tmp_value;

    // Handle 0 explicitly
    if (num == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return;
    }

    int is_negative = 0;
    if (num < 0 && base == 10) {
        is_negative = 1;
        num = -num;
    }

    while (num != 0) {
        tmp_value = num % base;
        if (tmp_value < 10) {
            *ptr++ = (char)(tmp_value + '0');
        } else {
            *ptr++ = (char)(tmp_value - 10 + 'a');
        }
        num /= base;
    }

    if (is_negative) {
        *ptr++ = '-';
    }

    *ptr-- = '\0';

    // Reverse the string by in-place swapping. ptr1 points to the start, ptr points to the end.
    // head -------- tail
    //  ^    swap     ^
    //  |_____________|
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}

static void handle_format_specifier(char specifier, va_list args) {
    char buffer[32];

    switch (specifier) {
        case 's': {
            const char* str = va_arg(args, const char*);
            while (*str) {
                print_char(*str++, 0x0F, 0x00); // White on black
            }
            break;
        }
        case 'c': {
            char c = (char)va_arg(args, int);
            print_char(c, 0x0F, 0x00); // White on black
            break;
        }
        case 'd': {
            int value = va_arg(args, int);
            itoa(value, buffer, 10);
            char* str = buffer;
            while (*str) {
                print_char(*str++, 0x0F, 0x00); // White on black
            }
            break;
        }
        case 'u': { // Unsigned integer
            unsigned int value = va_arg(args, unsigned int);
            itoa(value, buffer, 10);
            char* str = buffer;
            while (*str) {
                print_char(*str++, 0x0F, 0x00); // White on black
            }
            break;
        }
        case 'x': {
            int value = va_arg(args, int);
            itoa(value, buffer, 16);
            char* str = buffer;
            while (*str) {
                print_char(*str++, 0x0F, 0x00); // White on black
            }
            break;
        }
        case 'X': { // Unsigned hexadecimal (uppercase)
            unsigned int value = va_arg(args, unsigned int);
            itoa(value, buffer, 16);
            char* str = buffer;
            while (*str) {
                print_char(toupper(*str++), 0x0F, 0x00); // White on black
            }
            break;
        }
        case 'p': { // Pointer
            void* ptr = va_arg(args, void*);
            print_string("0x", 0x0F, 0x00); // White on black
            itoa((uintptr_t)ptr, buffer, 16);
            print_string(buffer, 0x0F, 0x00); // White on black
            break;
        }
        default:
            print_char('%', 0x0F, 0x00); // Print unknown specifier as is
            print_char(specifier, 0x0F, 0x00); // Print unknown specifier as is
            break;
    }
}

/**
 * A simple printf implementation that only supports string literals.
 * @param format The string to print.
 * 
 * TODO: Extend this function to support format specifiers and variable arguments.
 */
void printf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    const char* ptr = format;

    while (*ptr) {
        if (*ptr == '%') {
            ptr++;
            handle_format_specifier(*ptr, args);
            ptr++;
        }
        else {
            print_char(*ptr++, 0x0F, 0x00); // White on black
        }
    }

    va_end(args);
}

void clear_screen() {
    for (int i = 0; i < VIDEO_WIDTH * VIDEO_HEIGHT; i++) {
        video_memory[i] = 0x0F00; // White foreground, black background
    }
    vx = 0;
    vy = 0;
    disable_cursor();
}