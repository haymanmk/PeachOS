#include "classic.h"
#include "status.h"
#include "io/io.h"
#include "keyboard/keyboard.h"
#include "idt/idt.h"
#include <stddef.h>
#include <stdint.h>

/**
 * @file classic.c
 * @brief Classic PS/2 keyboard driver implementation.
 */

// Scancode set 1 mapping (partial)
// Refer to https://wiki.osdev.org/PS/2_Keyboard#Scan_Code_Set_1
static uint8_t scancode_set_1[] = {
    0x00, 0x1B, '1', '2', '3', '4', '5',
    '6', '7', '8', '9', '0', '-', '=',
    0x08, '\t', 'Q', 'W', 'E', 'R', 'T',
    'Y', 'U', 'I', 'O', 'P', '[', ']',
    0x0d, 0x00, 'A', 'S', 'D', 'F', 'G',
    'H', 'J', 'K', 'L', ';', '\'', '`', 
    0x00, '\\', 'Z', 'X', 'C', 'V', 'B',
    'N', 'M', ',', '.', '/', 0x00, '*',
    0x00, 0x20, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, '7', '8', '9', '-', '4', '5',
    '6', '+', '1', '2', '3', '0', '.'
};

int classic_keyboard_init();
void* classic_keyboard_handle_interrupt(idt_interrupt_stack_frame_t* frame);

keyboard_driver_t classic_keyboard_driver = {
    .init = classic_keyboard_init,
    .name = "Classic Keyboard Driver",
    .next = NULL
};

int classic_keyboard_init() {
    // Initialize the classic I8042 PS/2 Controller
    // Refer to https://wiki.osdev.org/I8042_PS/2_Controller for more details
    // Register keyboard interrupt handler in IDT
    idt_register_interrupt_handler(KEYBOARD_IDT_INTERRUPT_NUMBER, classic_keyboard_handle_interrupt);
    io_outb(CLASSIC_I8042_COMMAND_PORT, CLASSIC_I8042_ENABLE_FIRST_PORT);
    return ENONE;
}

uint8_t classic_scancode_to_ascii(uint8_t scancode) {
    // Convert scancode to ASCII using scancode set 1
    if (scancode < sizeof(scancode_set_1)) {
        return scancode_set_1[scancode];
    }
    return 0; // Unknown scancode
}

void* classic_keyboard_handle_interrupt(idt_interrupt_stack_frame_t* frame) {
    // Handle keyboard interrupt
    // We are in kernel mode and paging here
    // Read scancode from data port
    uint8_t scancode = io_inb(CLASSIC_I8042_DATA_PORT);
    uint8_t ascii = classic_scancode_to_ascii(scancode);

    // Ignore key releases
    if (scancode & CLASSIC_I8042_KEYBOARD_RELEASED_MASK) {
        return NULL;
    }

    if (ascii != 0) {
        keyboard_push((char)ascii);
    }
    return NULL;
}

keyboard_driver_t* classic_keyboard_driver_init() {
    // Initialize and return a pointer to the classic keyboard driver
    return &classic_keyboard_driver;
}