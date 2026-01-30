#include "peachos.h"

/**
 * @brief Get a character from the keyboard input, blocking until a character is available.
 * @return The character read from the keyboard.
 */
int peachos_getchar_blocking() {
    int value = 0;
    do {
        value = (int)peachos_getchar();
    } while (value == 0);
    return value;
}

/**
 * @brief Read a line of input from the terminal into the provided buffer.
 * @param buffer The buffer to store the input line.
 * @param max_length The maximum length of the input line (including null terminator).
 * @param echo Whether to echo the input characters to the terminal.
 */
void peachos_terminal_readline(char* buffer, size_t max_length, bool echo) {
    size_t index = 0;
    while (index < max_length - 1) { // Leave space for null terminator
        char c = peachos_getchar_blocking();
        if (c == '\n' || c == '\r') {
            if (echo) {
                peachos_putchar('\n');
            }
            break;
        } else if (c == '\b') { // Handle backspace
            if (index <= 0) {
                continue; // Nothing to backspace
            }
            index--;
            if (echo) {
                peachos_putchar('\b'); // Move cursor back
                peachos_putchar(' '); // Erase character
                peachos_putchar('\b'); // Move cursor back again
            }
        } else {
            buffer[index++] = c;
            if (echo) {
                peachos_putchar(c);
            }
        }
    }
    buffer[index] = '\0'; // Null-terminate the string
}