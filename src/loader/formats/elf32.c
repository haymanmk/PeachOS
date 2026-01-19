#include "elf32.h"

/**
 * Get the entry point address as a pointer from the ELF header.
 * @param elf_header Pointer to the ELF32 header.
 * @return Pointer to the entry point address, or NULL if elf_header is NULL.
 */
void* elf32_get_entry_pointer(elf32_header_t* elf_header) {
    if (!elf_header) {
        return NULL;
    }
    return (void*)(uintptr_t)(elf_header->e_entry);
}

/**
 * Get the entry point address from the ELF header.
 * @param elf_header Pointer to the ELF32 header.
 * @return Entry point address, or 0 if elf_header is NULL.
 */
uint32_t elf32_get_entry(elf32_header_t* elf_header) {
    if (!elf_header) {
        return 0;
    }
    return elf_header->e_entry;
}