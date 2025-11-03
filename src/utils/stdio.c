#include "stdio.h"

uint16_t *video_memory = (uint16_t *)0xB8000;
uint16_t vx = 0;
uint16_t vy = 0;

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

size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}

void printf(const char *str) {
    size_t len = strlen(str);
    for (size_t i = 0; i < len; i++) {
        print_char(str[i], 0x0F, 0x00); // White on black
    }
}

void clear_screen() {
    for (int i = 0; i < VIDEO_WIDTH * VIDEO_HEIGHT; i++) {
        video_memory[i] = 0x0F00; // White foreground, black background
    }
    vx = 0;
    vy = 0;
}