#include "kernel.h"
#include "utils/stdio.h"
#include "idt/idt.h"
#include "io/io.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"

static paging_4gb_chunk_t* kernel_paging_chunk = NULL;

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

    // Setup paging
    uint8_t paging_flags = PAGING_FLAG_PRESENT | PAGING_FLAG_WRITABLE | PAGING_FLAG_USER;
    kernel_paging_chunk = paging_4gb_chunk_init(paging_flags);

    // Switch to the new paging chunk
    paging_switch_4gb_chunk(kernel_paging_chunk);

    // Mpping a virtual address to a page frame
    uint32_t test_virtual_address = 0x00400000; // 4 MB
    // Allocate a page frame from the kernel heap
    paging_descriptor_entry_t* test_page_frame = (paging_descriptor_entry_t*)kheap_zmalloc(PAGE_SIZE);
    int map_result = paging_map_virtual_address(kernel_paging_chunk, test_virtual_address, (uint32_t)test_page_frame | paging_flags);
    if (map_result != ENONE) {
        printf("Failed to map virtual address 0x%X\n", test_virtual_address);
        return;
    }

    // Enable paging
    paging_enable();

    /**
     * At this point, paging is enabled. The kernel can now use virtual memory.
     */

    /**
     * Testing memory access after enabling paging.
     */
    char* test_ptr = (char*)test_virtual_address;
    test_ptr[0] = 'A';
    test_ptr[1] = 'B';
    printf(test_ptr);

    printf((const char*)test_page_frame);

    // Kernel main function implementation
    while (1) {
        // Kernel loop
    }
}