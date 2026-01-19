#ifndef __ELF32_LOADER_H__
#define __ELF32_LOADER_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "elf32.h"
#include "config.h"

typedef struct {
    char filename[FS_MAX_FILE_NAME_LENGTH];
    int in_memory_size;
    /**
     * Pointer to the ELF data in memory.
     * @note This is implemented as physical address.
     */
    void* elf_data;
    void* virtual_base_address;
    void* virtual_end_address;
    void* physical_base_address;
    void* physical_end_address;
} elf32_loader_file_t;

#define ELF32_LOADER_MIN_ALIGN PAGE_SIZE

#define ELF32_LOADER_PAGE_START(address) ((void*)((uintptr_t)(address) & ~(ELF32_LOADER_MIN_ALIGN - 1)))
#define ELF32_LOADER_PAGE_END(address) ((void*)(((uintptr_t)(address) + ELF32_LOADER_MIN_ALIGN - 1) & ~(ELF32_LOADER_MIN_ALIGN - 1)))
#define ELF32_LOADER_PAGE_OFFSET(address) ((uintptr_t)(address) & (ELF32_LOADER_MIN_ALIGN - 1))

#define ELF32_LOADER_ELF_HEADER(elf_file) ((elf32_header_t*)(elf_file->elf_data))
#define ELF32_LOADER_PROGRAM_HEADER_TABLE(elf_file) \
    ((elf32_program_header_t*)((uintptr_t)(elf_file->elf_data) + ELF32_LOADER_ELF_HEADER(elf_file)->e_phoff))

int elf32_loader_load(const char* filename, elf32_loader_file_t** out_elf_file);
void elf32_loader_close(elf32_loader_file_t* elf_file);

#endif // __ELF32_LOADER_H__