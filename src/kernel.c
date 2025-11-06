#include "kernel.h"
#include "utils/stdio.h"
#include "idt/idt.h"
#include "io/io.h"

void kernel_main() {
    clear_screen();
    printf("Welcome to PeachOS!\n");
    printf("Kernel initialized successfully.\n");

    // Initialize the Interrupt Descriptor Table (IDT)
    idt_init();

    // Enable interrupts
    idt_enable_interrupts();

    // Kernel main function implementation
    while (1) {
        // Kernel loop
    }
}