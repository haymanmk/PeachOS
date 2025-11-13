#ifndef __STDIO_H__
#define __STDIO_H__

#include <stddef.h>
#include <stdint.h>

#define VIDEO_WIDTH 80
#define VIDEO_HEIGHT 25

// Function prototypes
void put_char(int x, int y, char c, uint8_t fg, uint8_t bg);
void print_char(char c, uint8_t fg, uint8_t bg);
void printf(const char *str, ...);
void clear_screen();

#endif // __STDIO_H__