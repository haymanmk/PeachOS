#include "memory/paging/paging.h"
#include "memory/heap/kheap.h"

extern void paging_load_directory(paging_descriptor_entry_t* directory);

paging_4gb_chunk_t* paging_current_chunk = NULL;

/**
 * @brief Get the address of the page directory from a 4GB paging chunk.
 * @param chunk Pointer to the paging 4GB chunk.
 * @return Address of the page directory.
 */
static inline paging_descriptor_entry_t* paging_4gb_chunk_get_directory_address(paging_4gb_chunk_t* chunk) {
    return chunk->directory_ptr;
}

/**
 * @brief Check if an address is aligned to the page size.
 * @param address The address to check.
 * @return true if the address is aligned to the page size, false otherwise.
 */
bool paging_is_aligned_to_page_size(uint32_t address) {
    return (address % PAGE_SIZE) == 0;
}

/**
 * @brief Get the page directory and table indexes from a virtual address.
 * @param virtual_address The virtual address to translate.
 * @param directory_index Pointer to store the resulting page directory index.
 * @param table_index Pointer to store the resulting page table index.
 * @return ENONE on success, or -EINVAL if the address is not aligned to page size.
 */
int paging_get_indexes_from_address(uint32_t virtual_address, uint32_t* directory_index, uint32_t* table_index) {
    if (!paging_is_aligned_to_page_size(virtual_address)) {
        return -EINVAL; // Address is not aligned to page size
    }

    *directory_index = virtual_address / (PAGE_SIZE * PAGE_ENTRIES_PER_TABLE); // 4MB per directory entry
    *table_index = (virtual_address % (PAGE_SIZE * PAGE_ENTRIES_PER_TABLE)) / PAGE_SIZE; // 4KB per page

    return ENONE;
}

/**
 * @brief Initialize a 4GB paging chunk with 4KB pages.
 */
paging_4gb_chunk_t* paging_4gb_chunk_init(uint8_t flags) {
    // Allocate a chunk structure
    paging_4gb_chunk_t* chunk = (paging_4gb_chunk_t*)kheap_zmalloc(sizeof(paging_4gb_chunk_t));
    if (!chunk) {
        // Handle allocation failure (e.g., log it, halt the system, etc.)
        return NULL;
    }

    // prepare page directory table
    paging_descriptor_entry_t* page_directory = (paging_descriptor_entry_t*)kheap_zmalloc(PAGE_DIRECTORY_SIZE);
    if (!page_directory) {
        // Handle allocation failure (e.g., log it, halt the system, etc.)
        goto cleanup;
    }

    // Map descriptors to page tables
    for (uint32_t i = 0; i < PAGE_ENTRIES_PER_TABLE; i++) {
        // Allocate a page table
        paging_descriptor_entry_t* page_table = (paging_descriptor_entry_t*)kheap_zmalloc(PAGE_TABLE_SIZE);
        if (!page_table) {
            // Handle allocation failure (e.g., log it, halt the system, etc.)
            page_directory = NULL;
            goto cleanup;
        }

        // Map the page table to the page directory
        page_directory[i] = (paging_descriptor_entry_t)page_table | flags;

        // Initialize the page table entries
        for (uint32_t j = 0; j < PAGE_ENTRIES_PER_TABLE; j++) {
            page_table[j] = ((i * PAGE_ENTRIES_PER_TABLE * PAGE_SIZE) + (j * PAGE_SIZE)) | flags;
        }
    }

    chunk->directory_ptr = page_directory;

    return chunk;

// Free allocated resources to prevent memory leaks
cleanup:
    if (page_directory) {
        for (uint32_t i = 0; i < PAGE_ENTRIES_PER_TABLE; i++) {
            if (page_directory[i]) {
                kheap_free((void*)(page_directory[i] & ~0xFFF));
            }
        }
    }

    if (chunk) {
        kheap_free((void*)chunk);
    }
    return NULL;
}

/**
 * @brief Switch to a different 4GB paging chunk.
 * @param chunk Pointer to the paging 4GB chunk to switch to.
 */
void paging_switch_4gb_chunk(paging_4gb_chunk_t* chunk) {
    paging_descriptor_entry_t* directory_address = paging_4gb_chunk_get_directory_address(chunk);
    paging_load_directory(directory_address);
    paging_current_chunk = chunk;
}

/**
 * @brief Map a virtual address to a page frame in the given paging chunk.
 * @param chunk Pointer to the paging 4GB chunk.
 * @param virtual_address The virtual address to map (should be aligned to page size).
 * @param value The pointer to the page frame descriptor with flags.
 * @return ENONE on success, or -EINVAL on failure.
 */
int paging_map_virtual_address(
    paging_4gb_chunk_t* chunk,
    uint32_t virtual_address,
    uint32_t value) {
    if (!chunk || !value) {
        return -EINVAL;
    }

    uint32_t directory_index, table_index;
    if (paging_get_indexes_from_address(virtual_address, &directory_index, &table_index) != ENONE) {
        return -EINVAL;
    }

    // Remove the flags from the page table address first
    paging_descriptor_entry_t* page_table =
        (paging_descriptor_entry_t*)(chunk->directory_ptr[directory_index] & ~0xFFF);
    if (!page_table) {
        return -EINVAL;
    }
    page_table[table_index] = value;

    return ENONE;
}

