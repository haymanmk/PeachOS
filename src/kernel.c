#include "kernel.h"
#include "utils/stdio.h"
#include "idt/idt.h"
#include "io/io.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "disk/disk.h"
#include "disk/streamer.h"
#include "fs/pparser.h"

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

    // Initialize file system module
    if (file_init() != ENONE) {
        printf("File system initialization failed!\n");
        return;
    }

    // Initialize disk subsystem
    if (disk_init() != 0) {
        printf("Disk initialization failed!\n");
        return;
    }

    /**
     * Testing disk read
     */
    disk_t* disk0 = disk_get_by_uid(0);
    if (disk0) {
        uint8_t buffer[DISK_SECTOR_SIZE]; // Buffer to hold one sector
        if (disk_read_lba(disk0, 0, 1, buffer) == 0) {
            printf("Read sector 0 from disk 0 successfully.\n");
            printf("First byte of sector: \r\n");
        }
    }

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
     * Testing functionality.
     */
    // test writing to the mapped virtual address
    char* test_ptr = (char*)test_virtual_address;
    test_ptr[0] = 'A';
    test_ptr[1] = 'B';
    printf(test_ptr);

    printf((const char*)test_page_frame);

    // test reading from streamer
    disk_streamer_t* streamer = disk_streamer_create(0);
    if (!streamer) {
        printf("Failed to create disk streamer.\n");
        return;
    }
    if (disk_streamer_seek(streamer, 0x201) != 0) {
        printf("Failed to seek in disk streamer.\n");
        return;
    }
    uint8_t read_buffer[16];
    if (disk_streamer_read(streamer, sizeof(read_buffer), read_buffer) != 0) {
        printf("Failed to read from disk streamer.\n");
        return;
    }
    printf("Data read from disk streamer:\n");

    // test path parser
    const char* test_path = "0:/config.d/sub/config.yml";
    path_root_t* parsed_path = path_parse(test_path);
    if (parsed_path) {
        printf("Parsed path for drive number: %d\n", parsed_path->drive_no);
    }

    // test file open
    int fd = file_open(test_path, "r");
    if (fd < 0) {
        printf("Failed to open file: %s\n", test_path);
    } else {
        printf("File opened successfully with descriptor: %d\n", fd);
        uint8_t file_read_buffer[16];
        size_t items_read = file_read(file_read_buffer, 1, sizeof(file_read_buffer)-1, fd);
        if (items_read > 0) {
            file_read_buffer[items_read] = '\0'; // Null-terminate the buffer
            printf((const char*)file_read_buffer);
        }
    }

    // Kernel main function implementation
    while (1) {
        // Kernel loop
    }
}