#include "kernel.h"
#include "utils/stdio.h"
#include "idt/idt.h"
#include "io/io.h"
#include "memory/heap/kheap.h"

void kernel_main() {
    clear_screen();
    printf("Welcome to PeachOS!\n");
    printf("Kernel initialized successfully.\n");

    // Initialize the Interrupt Descriptor Table (IDT)
    idt_init();

    // Enable interrupts
    idt_enable_interrupts();

    // Initialize the kernel heap
    kheap_init();

    // Malloc some memory from the kernel heap for testing
    void* test_ptr = kheap_malloc(KERNEL_HEAP_BLOCK_SIZE * 3);
    (void)test_ptr; // Suppress unused variable warning

    // Kernel main function implementation
    while (1) {
        // Kernel loop
    }
}