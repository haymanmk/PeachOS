#include <stdbool.h>
#include "heap.h"
#include "memory/memory.h"
#include "utils/stdio.h"

/**
 * @file heap.c
 * @brief General heap memory management implementation.
 */

#define HEAP_INVALID_BLOCK_INDEX (uint32_t)0xFFFFFFFF

 /**
  * @brief Validate the heap block table.
  * @param table Pointer to the heap block table.
  * @param start_ptr Start address of the heap.
  * @param end_ptr End address of the heap.
  * @return ENONE if valid, error code otherwise.
  */
static error_t heap_validate_table(heap_table_t* table, void* start_ptr, void* end_ptr) {
    size_t total_size = (size_t)((uintptr_t)end_ptr - (uintptr_t)start_ptr);
    size_t expected_blocks = total_size / KERNEL_HEAP_BLOCK_SIZE;
    if (table->total_blocks != expected_blocks) {
        return -EINVALARG;
    }
    return ENONE;
}

 /**
  * @brief Validate if a pointer is aligned to the heap block size.
  * @param ptr Pointer to validate.
  * @return true if aligned, false otherwise.
  */
static bool heap_validate_alignment(void* ptr) {
    return ((intptr_t)ptr % KERNEL_HEAP_BLOCK_SIZE) == 0;
}

/**
 * @brief Find the first free block index of a chunk of contiguous blocks in the heap for allocation.
 * @param heap Pointer to the heap structure.
 * @param num_blocks Number of contiguous blocks needed.
 * @return The starting block index if found, HEAP_INVALID_BLOCK_INDEX otherwise.
 */
static uint32_t heap_get_start_block_index(heap_t* heap, uint32_t num_blocks) {
    uint32_t bs = HEAP_INVALID_BLOCK_INDEX; // block start
    uint32_t bc = 0; // block count
    heap_table_t* table = heap->table;

    // Iterate through the heap block entries to find a suitable chunk
    for (uint32_t i = 0; i < table->total_blocks; i++) {
        heap_block_entry_t entry = table->entries[i];
        if (HEAP_GET_ENTRY_TYPE(entry) != HEAP_BLOCK_TYPE_FREE) {
            // Reset if a used/reserved block is encountered.
            // This manner ensures only contiguous free blocks are considered.
            bs = HEAP_INVALID_BLOCK_INDEX;
            bc = 0;
            continue;
        }
        if (bs == HEAP_INVALID_BLOCK_INDEX) {
            bs = i; // Mark the start of a potential chunk
        }
        bc++;
        if (bc == num_blocks) {
            // Found a suitable chunk
            break;
        }
    }

    return bs;
}

/**
 * @brief Get the memory address of a specific block index in the heap.
 * @param heap Pointer to the heap structure.
 * @param block_index The block index.
 * @return The memory address of the block.
 */
static void* heap_get_block_address(heap_t* heap, uint32_t block_index) {
    return (void*)((uintptr_t)heap->start_address + (block_index * KERNEL_HEAP_BLOCK_SIZE));
}

/**
 * @brief Get the block index corresponding to a given memory pointer in the heap.
 * @param heap Pointer to the heap structure.
 * @param ptr The memory pointer.
 * @return The block index.
 */
static uint32_t heap_get_block_index(heap_t* heap, void* ptr) {
    return (uint32_t)(((uintptr_t)ptr - (uintptr_t)heap->start_address) / KERNEL_HEAP_BLOCK_SIZE);
}

/**
 * @brief Mark a range of blocks as used in the heap block table.
 * @param heap Pointer to the heap structure.
 * @param start_block The starting block index.
 * @param num_blocks The number of blocks to mark as used.
 */
static void heap_mark_blocks_used(heap_t* heap, uint32_t start_block, uint32_t num_blocks) {
    heap_table_t* table = heap->table;
    for (uint32_t i = 0; i < num_blocks; i++) {
        uint8_t entry = HEAP_BLOCK_TYPE_USED;
        if (i == 0) {
            entry |= HEAP_BLOCK_FLAG_IS_FIRST; // IS_FIRST
        }
        if (i < num_blocks - 1) {
            entry |= HEAP_BLOCK_FLAG_HAS_NEXT; // HAS_NEXT
        }
        table->entries[start_block + i] = entry;
    }
}

/**
 * @brief Mark a range of blocks as free in the heap block table.
 * @param heap Pointer to the heap structure.
 * @param start_block The starting block index.
 */
static void heap_mark_blocks_free(heap_t* heap, uint32_t start_block) {
    heap_table_t* table = heap->table;
    uint32_t current_block = start_block;

    while (true) {
        heap_block_entry_t entry = table->entries[current_block];
        table->entries[current_block] = HEAP_BLOCK_TYPE_FREE; // Mark as free

        // Check if this block has a next block
        if (entry & HEAP_BLOCK_FLAG_HAS_NEXT) {
            current_block++;
        } else {
            break; // No more blocks in this allocation
        }
    }
}

/**
 * @brief Align a value to the upper multiple of the heap block size.
 * @param val The value to align.
 * @return The aligned value.
 */
static uint32_t heap_align_value_to_upper(size_t val) {
    if (val % KERNEL_HEAP_BLOCK_SIZE == 0) {
        return (uint32_t)val;
    }
    return (uint32_t)((val / KERNEL_HEAP_BLOCK_SIZE) + 1) * KERNEL_HEAP_BLOCK_SIZE;
}

/**
 * @brief Allocate a chunk of contiguous blocks in the heap.
 * @param heap Pointer to the heap structure.
 * @param num_blocks Number of contiguous blocks needed.
 * @return Pointer to the starting address of the allocated memory, or NULL if allocation fails.
 */
static void* heap_malloc_blocks(heap_t* heap, uint32_t num_blocks) {
    uint32_t start_block = heap_get_start_block_index(heap, num_blocks);
    if (start_block == HEAP_INVALID_BLOCK_INDEX) {
        return NULL; // No suitable chunk found
    }

    // Mark the blocks as used
    heap_mark_blocks_used(heap, start_block, num_blocks);

    // Return the starting address of the allocated memory
    return heap_get_block_address(heap, start_block);
}

/**
  * @brief Initialize the general heap.
  * @param heap Pointer to the heap structure to initialize.
  * @param start_ptr Start address of the heap.
  * @param end_ptr End address of the heap.
  * @param table Pointer to the heap block table.
  * @return ENONE if successful, error code otherwise (< 0).
  */
error_t heap_init(heap_t* heap, void* start_ptr, void* end_ptr, heap_table_t* table) {
    error_t err = ENONE;
    // Validate the heap block table
    if ((err = heap_validate_table(table, start_ptr, end_ptr)) != ENONE) {
        goto exit;
    }

    // Validate the start and end pointers alignment
    if (!heap_validate_alignment(start_ptr) || !heap_validate_alignment(end_ptr)) {
        err = -EINVALARG;
        goto exit;
    }

    // Initialize the heap structure
    heap->table = table;
    heap->start_address = start_ptr;

    // Mark all blocks as free in the heap block table
    size_t table_size = table->total_blocks * sizeof(heap_block_entry_t);
    memset((void*)table->entries, 0, table_size);

exit:
    return err;
}

/**
 * @brief Allocate memory from the general heap.
 * @param heap Pointer to the heap structure.
 * @param size The size of memory to allocate in bytes.
 * @return Pointer to the allocated memory, or NULL if allocation fails.
 */
void* heap_malloc(heap_t* heap, size_t size) {
    // Align the size to the upper multiple of the heap block size
    size = heap_align_value_to_upper(size);

    // Calculate the number of blocks needed
    uint32_t num_blocks = size / KERNEL_HEAP_BLOCK_SIZE;

    // Allocate the blocks
    return heap_malloc_blocks(heap, num_blocks);
}

/**
 * @brief Free memory back to the general heap.
 * @param heap Pointer to the heap structure.
 * @param ptr Pointer to the memory to free.
 */
void heap_free(heap_t* heap, void* ptr) {
    // Validate the pointer alignment
    if (!heap_validate_alignment(ptr)) {
        printf("Invalid pointer alignment: %p\n", ptr);

        /* TODO: Handle error (e.g., errcode, exception) */

        return; // Invalid pointer, do nothing
    }

    // Get the block index corresponding to the pointer
    uint32_t block_index = heap_get_block_index(heap, ptr);

    // Mark the blocks as free
    heap_mark_blocks_free(heap, block_index);
}