#ifndef __HEAP_H__
#define __HEAP_H__

#include <stddef.h>
#include <stdint.h>
#include "status.h"
#include "config.h"

/**
 * The workflow of how the heap works is as follows:
 * 1. The heap is divided into fixed-size blocks (e.g., 4 KB).
 * 2. A heap block table is maintained to track the status of each block (free or used).
 * 3. When memory is allocated, the heap searches for a contiguous sequence of free blocks
 *    that can satisfy the requested size.
 * 4. The corresponding entries in the heap block table are updated to mark these blocks as used.
 * 5. When memory is freed, the corresponding blocks are marked as free in the heap block table.
 * 
 * The heap structure tracks a pointer to the heap block table instead of allocating memory directly,
 * allowing for flexibly managing different heap sizes and locations.
 * 
 * Heap entry layout:
 * +-------------------------------------------------------------------------+
 * | bit 7    | bit 6    | bit 5 | bit 4 | bit 3  | bit 2  | bit 1  | bit 0  |
 * +-------------------------------------------------------------------------+
 * | HAS_NEXT | IS_FIRST | 0     | 0     | TYPE_3 | TYPE_2 | TYPE_1 | TYPE_0 |
 * +-------------------------------------------------------------------------+
 * HAS_NEXT: Indicates if there is a next block in the allocation.
 * IS_FIRST: Indicates if this block is the first block in the allocation.
 * TYPE_x:   Represents the type of the block (e.g., free, used, reserved).
 *           - 0000: Free block
 *           - 0001: Used block
 *           - 0010: Reserved block
 */

#define HEAP_BLOCK_TYPE_FREE     0x0
#define HEAP_BLOCK_TYPE_USED     0x1
#define HEAP_BLOCK_TYPE_RESERVED 0x2 
#define HEAP_BLOCK_TYPE_MASK     0x0F
#define HEAP_BLOCK_FLAG_IS_FIRST_MASK (0x01 << 6)
#define HEAP_BLOCK_FLAG_IS_FIRST HEAP_BLOCK_FLAG_IS_FIRST_MASK
#define HEAP_BLOCK_FLAG_HAS_NEXT_MASK (0x01 << 7)
#define HEAP_BLOCK_FLAG_HAS_NEXT HEAP_BLOCK_FLAG_HAS_NEXT_MASK

// Type definition for heap implementation
// used to represent the entry pointer in the heap block table
typedef uint8_t heap_block_entry_t;

// Structure representing the heap block table
typedef struct heap_table {
    heap_block_entry_t* entries;
    uint32_t total_blocks;
} heap_table_t;

// Structure representing the heap
typedef struct heap {
    heap_table_t* table;
    void* start_address;
} heap_t;

// Macros
#define HEAP_GET_ENTRY_TYPE(entry) ((entry) & 0x0F)

// Heap management functions
error_t heap_init(heap_t* heap, void* start_ptr, void* end_ptr, heap_table_t* table);
void* heap_malloc(heap_t* heap, size_t size);
void heap_free(heap_t* heap, void* ptr);

#endif // __HEAP_H__
