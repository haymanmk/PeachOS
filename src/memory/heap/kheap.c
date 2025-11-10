#include "kheap.h"
#include "heap.h"
#include "utils/stdio.h"
#include "memory/memory.h"

/**
 * @file kheap.c
 * @brief Kernel heap memory management implementation.
 */

static heap_t kernel_heap;
static heap_table_t kernel_heap_table;

/**
 * @brief Initialize the kernel heap.
 */
void kheap_init() {
    void* heap_start = (void*)KERNEL_HEAP_ADDRESS;
    void* heap_end = (void*)((uintptr_t)KERNEL_HEAP_ADDRESS + KERNEL_HEAP_SIZE_BYTES);
    void* table_address = (void*)KERNEL_HEAP_TABLE_ADDRESS;

    // Initialize the kernel heap block table
    kernel_heap_table.entries = (heap_block_entry_t*)table_address;
    kernel_heap_table.total_blocks = KERNEL_HEAP_MAX_BLOCKS;

    // Initialize the kernel heap
    error_t err = heap_init(&kernel_heap, heap_start, heap_end, &kernel_heap_table);
    if (err != ENONE) {
        // Handle initialization error (e.g., log it)
        printf("Kernel heap initialization failed with error code: %d\n", err);
    }
}

/**
 * @brief Allocate memory from the kernel heap.
 * @param size The size of memory to allocate in bytes.
 * @return Pointer to the allocated memory, or NULL if allocation fails.
 */
void* kheap_malloc(size_t size) {
    return heap_malloc(&kernel_heap, size);
}

/**
 * @brief Allocate zero-initialized memory from the kernel heap.
 * @param size The size of memory to allocate in bytes.
 * @return Pointer to the allocated memory, or NULL if allocation fails.
 */
void* kheap_zmalloc(size_t size) {
    void* ptr = kheap_malloc(size);
    if (ptr) {
        memset(ptr, 0, size);
    }
    return ptr;
}

/**
 * @brief Free memory back to the kernel heap.
 * @param ptr Pointer to the memory to free.
 */
void kheap_free(void* ptr) {
    heap_free(&kernel_heap, ptr);
}