#include "kernel.h"
#include "utils/stdio.h"
#include "idt/idt.h"
#include "io/io.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "memory/memory.h"
#include "disk/disk.h"
#include "disk/streamer.h"
#include "fs/pparser.h"
#include "gdt/gdt.h"
#include "config.h"
#include "task/tss.h"
#include "task/process.h"
#include "isr80h/isr80h.h"
#include "keyboard/keyboard.h"

static paging_4gb_chunk_t* kernel_paging_chunk = NULL;

tss_t tss;

gdt_entry_t gdt_entries[GDT_MAX_ENTRIES];
gdt_structured_t structured_gdt[GDT_MAX_ENTRIES] = {
    // Null segment
    {.base = 0, .limit = 0, .type = 0},
    // Kernel code segment
    {.base = 0, .limit = 0xFFFFFFFF, .type = 0x9A},
    // Kernel data segment
    {.base = 0, .limit = 0xFFFFFFFF, .type = 0x92},
    // User code segment
    {.base = 0, .limit = 0xFFFFFFFF, .type = 0xFA},
    // User data segment
    {.base = 0, .limit = 0xFFFFFFFF, .type = 0xF2},
    // TSS segment
    {.base = (uint32_t)&tss, .limit = sizeof(tss)-1, .type = 0xE9}
};

void panic(const char* message) {
    printf("KERNEL PANIC: %s\n", message);
    while (1) {
        // Halt the CPU
        __asm__ __volatile__("hlt");
    }
}

void kernel_main() {
    clear_screen();
    printf("Welcome to PeachOS!\n");
    printf("Kernel initialized successfully.\n");

    // Initialize the Global Descriptor Table (GDT)
    // Reset GDT entries
    memset(&gdt_entries, 0, sizeof(gdt_entries));
    gdt_init(gdt_entries, structured_gdt, GDT_MAX_ENTRIES);
    gdt_load(gdt_entries, sizeof(gdt_entries) - 1);

    // Initialize the kernel heap
    kheap_init();

    // Initialize the Interrupt Descriptor Table (IDT)
    idt_init();

    // Enable interrupts
    idt_enable_interrupts();

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

    // Setup TSS
    memset(&tss, 0, sizeof(tss_t));
    tss.ss0 = KERNEL_DATA_SELECTOR;
    tss.esp0 = KERNEL_HEAP_ADDRESS + KERNEL_HEAP_SIZE_BYTES; // Stack pointer for kernel mode

    // Load TSS segment into the task register
    tss_load(0x28); // TSS segment selector is at index 5

    // Setup paging
    uint8_t paging_flags = PAGING_FLAG_PRESENT | PAGING_FLAG_WRITABLE | PAGING_FLAG_USER;
    kernel_paging_chunk = paging_4gb_chunk_init(paging_flags);

    // Switch to the new paging chunk
    paging_switch_4gb_chunk(kernel_paging_chunk);

    // Enable paging
    paging_enable();

    // Register ISR 0x80 commands
    if (isr80h_register_commands() != ENONE) {
        panic("Failed to register ISR 0x80 commands.");
    }

    // Initialize keyboard
    keyboard_init();

    /**
     * At this point, paging is enabled. The kernel can now use virtual memory.
     */

    // Test loading programs
    printf("Loading user program 'blank.bin'...\n");
    process_t* user_process = NULL;
    int load_result = process_load("0:/programs/blank.bin", &user_process);
    if (load_result != ENONE) {
        panic("Failed to load user program 'blank.bin'.");
    }
    printf("User program 'blank.bin' loaded successfully with PID %d.\n", user_process->pid);
    // Run the first ever task (user program)
    task_run_first_ever_task();

    // Kernel main function implementation
    while (1) {
        // Kernel loop
    }
}

void kernel_page() {
    if (!kernel_paging_chunk) {
        panic("Kernel paging chunk is not initialized.");
    }
    // Restore segment registers to the kernel data segment (DS, ES, FS, GS)
    kernel_restore_segment_registers_to_kernel_data();
    // Switch to the kernel paging chunk
    paging_switch_4gb_chunk(kernel_paging_chunk);
}