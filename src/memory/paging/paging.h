#ifndef __PAGING_H__
#define __PAGING_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "config.h"
#include "status.h"

#define PAGING_FLAG_PRESENT        0b00000001
#define PAGING_FLAG_WRITABLE       0b00000010
#define PAGING_FLAG_USER           0b00000100 // User-mode accessible
#define PAGING_FLAG_WRITE_THROUGH  0b00001000
#define PAGING_FLAG_CACHE_DISABLED 0b00010000
#define PAGING_FLAG_ACCESSED       0b00100000
#define PAGING_FLAG_DIRTY          0b01000000
#define PAGING_FLAG_PAGE_SIZE      0b10000000

// Type definitions
/**
 * @brief Paging descriptor type (page directory or page table entry).
 */
typedef uint32_t paging_descriptor_entry_t;
typedef struct paging_4gb_chunk {
    paging_descriptor_entry_t* directory_ptr; // Pointer to the page directory
} paging_4gb_chunk_t;

// Exported function prototypes
paging_4gb_chunk_t* paging_4gb_chunk_init(uint8_t flags);
void paging_switch_4gb_chunk(paging_4gb_chunk_t* chunk);
int paging_map_virtual_address(paging_4gb_chunk_t* chunk, uint32_t virtual_address, uint32_t value);
bool paging_is_aligned_to_page_size(uint32_t address);
void paging_align_address_to_page_size(uint32_t* address);
void paging_4gb_chunk_free(paging_4gb_chunk_t* chunk);
int paging_map_virtual_addresses(paging_4gb_chunk_t* chunk, uint32_t virtual_address_start, uint32_t physical_address_start, size_t size, uint32_t flags);
uint32_t paging_get_page_entry(paging_4gb_chunk_t* chunk, uint32_t virtual_address);

/**
 * @brief Enable paging by setting the appropriate control register.
 *        This function is typically implemented in assembly.
 */
void paging_enable();

#endif // __PAGING_H__