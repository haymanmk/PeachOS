#include "keyboard.h"
#include "task/process.h"
#include "task/task.h"
#include "status.h"
#include "classic.h"

/**
 * @file keyboard.c
 * @brief Keyboard handling implementation.
 * This file contains the implementation of keyboard functions,
 * including initialization and buffer management.
 * The buffer is implemented as a ring buffer for each process.
 * Due to each process having its own keyboard buffer,
 * keyboard input handling simply considers the current process's buffer.
 */

static keyboard_driver_t* keyboard_driver_list_head = NULL;
static keyboard_driver_t* keyboard_driver_list_tail = NULL;

uint32_t keyboard_increment_index(uint32_t index) {
    return (index + 1) % KEYBOARD_BUFFER_SIZE;
}

uint32_t keyboard_decrement_index(uint32_t index) {
    return (index + KEYBOARD_BUFFER_SIZE - 1) % KEYBOARD_BUFFER_SIZE;
}

uint32_t keyboard_increment_head(process_t* process) {
    process->keyboard.head = keyboard_increment_index(process->keyboard.head);
    return process->keyboard.head;
}

uint32_t keyboard_increment_tail(process_t* process) {
    process->keyboard.tail = keyboard_increment_index(process->keyboard.tail);
    return process->keyboard.tail;
}

uint32_t keyboard_decrement_head(process_t* process) {
    process->keyboard.head = keyboard_decrement_index(process->keyboard.head);
    return process->keyboard.head;
}

uint32_t keyboard_decrement_tail(process_t* process) {
    process->keyboard.tail = keyboard_decrement_index(process->keyboard.tail);
    return process->keyboard.tail;
}

/**
 * @brief Initialize the keyboard system including drivers.
 */
void keyboard_init() {
    // Initialize keyboard system including drivers
    keyboard_register_driver(classic_keyboard_driver_init());
}

int keyboard_register_driver(keyboard_driver_t* driver) {
    // Register a keyboard driver
    int res = ENONE;
    // Check if driver is valid
    if (!driver || !driver->init) {
        return -EINVAL;
    }
    // Initialize the driver
    res = driver->init();
    if (res < 0) {
        return res; // Propagate error code
    }
    // Add driver to the linked list
    driver->next = NULL; // Ensure next is NULL
    if (!keyboard_driver_list_head) {
        keyboard_driver_list_head = driver;
        keyboard_driver_list_tail = driver;
    } else {
        keyboard_driver_list_tail->next = driver;
        keyboard_driver_list_tail = driver;
    }
    return ENONE;
}

void keyboard_backspace() {
    // Handle backspace for the current process's keyboard buffer
    // Get the current process
    process_t* current_process = process_get_current();
    if (!current_process) {
        return; // No current process
    }
    // Modify the keyboard buffer of the current process
    if (current_process->keyboard.tail != current_process->keyboard.head) {
        // Decrement tail to remove last character
        keyboard_decrement_tail(current_process);
    }
}

void keyboard_push(char c) {
    // Push a character to the tail of keyboard buffer
    // Get the current process
    process_t* current_process = process_get_current();
    if (!current_process) {
        return; // No current process
    }

    // Ignore if c is null
    if (c == '\0') {
        return;
    }

    // Modify the keyboard buffer of the current process
    // Check if buffer is full
    if (keyboard_increment_index(current_process->keyboard.tail) == current_process->keyboard.head) {
        return; // Buffer is full, do not overwrite unread data
    }
    current_process->keyboard.buffer[current_process->keyboard.tail] = c;
    // Increment tail
    keyboard_increment_tail(current_process);
}

char keyboard_pop() {
    // Pop a character from the head of keyboard buffer
    // Get the current process
    process_t* current_process = process_get_current();
    if (!current_process) {
        return '\0'; // No current process
    }
    // Check if buffer is empty
    if (current_process->keyboard.head == current_process->keyboard.tail) {
        return '\0'; // Buffer is empty
    }
    // Get the character at head
    char c = current_process->keyboard.buffer[current_process->keyboard.head];
    // Increment head
    keyboard_increment_head(current_process);
    return c;
}