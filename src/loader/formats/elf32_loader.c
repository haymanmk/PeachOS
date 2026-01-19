#include "memory/heap/kheap.h"
#include "fs/file.h"
#include "status.h"
#include "elf32_loader.h"

/**
 * Check if the ELF file is a valid executable.
 * @param elf_header Pointer to the ELF32 header.
 * @return true if the ELF file is an executable, false otherwise.
 * @note This function assumes that elf_header is not NULL.
 *       It checks if the e_type is ET_EXEC and if the entry point
 *       is equal to or greater than PROGRAM_VIRTUAL_ADDRESS.
 */
static bool elf32_loader_is_executable(elf32_header_t* elf_header) {
    return ((elf_header->e_type == ET_EXEC) &&
            (elf_header->e_entry >= PROGRAM_VIRTUAL_ADDRESS));
}

/**
 * Check if the ELF file has a program header table.
 * @param elf_header Pointer to the ELF32 header.
 * @return true if the ELF file has a program header table, false otherwise.
 * @note This function assumes that elf_header is not NULL.
 *       It checks if e_phoff and e_phnum are non-zero.
 */
static bool elf32_loader_has_program_header_table(elf32_header_t* elf_header) {
    return (elf_header->e_phoff != 0 && elf_header->e_phnum != 0);
}

/**
 * @brief Calculate and update the load memory addresses for a program header.
 * @param elf_file Pointer to the elf32_loader_file_t structure.
 * @param pheader Pointer to the elf32_program_header_t structure.
 * @return ENONE on success, or a negative error code on failure.
 * @note This function updates the virtual and physical base and end addresses
 *       in the elf32_loader_file_t structure based on the provided program header.
 *       The true memory mapping will be handled later during process memory mapping.
 */
int elf32_loader_calculate_load_memory_addresses(elf32_loader_file_t* elf_file, elf32_program_header_t* pheader) {
    int res = 0;
    // Find the lowest virtual address and corresponding physical address
    if (elf_file->virtual_base_address >= (void*)pheader->p_vaddr ||
        elf_file->virtual_base_address == 0) {
        // Update virtual base address
        elf_file->virtual_base_address = (void*)(uintptr_t)pheader->p_vaddr;
        // Update physical base address
        elf_file->physical_base_address = ELF32_LOADER_ELF_HEADER(elf_file) + pheader->p_offset;
    }
    // Find the highest virtual address and corresponding physical address
    if (elf_file->virtual_end_address <= (void*)(uintptr_t)(pheader->p_vaddr + pheader->p_memsz) ||
        elf_file->virtual_end_address == 0) {
        // Update virtual end address
        elf_file->virtual_end_address = (void*)(uintptr_t)(pheader->p_vaddr + pheader->p_memsz);
        // Update physical end address
        elf_file->physical_end_address = ELF32_LOADER_ELF_HEADER(elf_file) + pheader->p_offset + pheader->p_memsz;
    }

    return res;
}

int elf32_loader_process_pheader(elf32_loader_file_t* elf_file, elf32_program_header_t* pheader) {
    int res = 0;
    // Process only loadable segments
    switch (pheader->p_type) {
        case PT_LOAD:
            // Calculate and update load memory addresses
            res = elf32_loader_calculate_load_memory_addresses(elf_file, pheader);
            // Note: Even though this function currently cannot fail, we still check the return value
            if (res < 0) {
                return res;
            }
            break;
    }
    return res;
}

int elf32_loader_process_pheaders(elf32_loader_file_t* elf_file) {
    int res = 0;
    elf32_header_t* elf_header = ELF32_LOADER_ELF_HEADER(elf_file);
    elf32_program_header_t* pheader_table = ELF32_LOADER_PROGRAM_HEADER_TABLE(elf_file);
    // Process each program header
    for (uint16_t i = 0; i < elf_header->e_phnum; i++) {
        elf32_program_header_t* pheader = &pheader_table[i];
        res = elf32_loader_process_pheader(elf_file, pheader);
        if (res < 0) {
            return res;
        }
    }
    
    return ENONE;
}

/**
 * @brief Validate the ELF header to ensure it is a proper ELF32 executable.
 * @param elf_header Pointer to the ELF32 header.
 * @return ENONE if valid, or a negative error code if invalid.
 */
int elf32_loader_validate_elf_header(elf32_header_t* elf_header) {
    if (!elf_header) {
        return -EINVAL;
    }
    // Validate ELF magic numbers
    if (elf_header->e_ident[EI_MAG0] != ELFMAG0 ||
        elf_header->e_ident[EI_MAG1] != ELFMAG1 ||
        elf_header->e_ident[EI_MAG2] != ELFMAG2 ||
        elf_header->e_ident[EI_MAG3] != ELFMAG3) {
        return -EINVAL; // Invalid ELF magic number
    }
    // Validate ELF class, accept None or 32-bit
    if (elf_header->e_ident[EI_CLASS] != ELFCLASSNONE &&
        elf_header->e_ident[EI_CLASS] != ELFCLASS32) {
        return -EINVAL; // Not a None or 32-bit ELF file
    }
    // Validate data encoding, accept None or little-endian
    if (elf_header->e_ident[EI_DATA] != ELFDATANONE &&
        elf_header->e_ident[EI_DATA] != ELFDATA2LSB) {
        return -EINVAL; // Not little-endian
    }
    // Validate that it's an executable ELF
    if (!elf32_loader_is_executable(elf_header)) {
        return -EINVAL; // Not an executable ELF
    }
    // Check if program header table exists
    if (!elf32_loader_has_program_header_table(elf_header)) {
        return -EINVAL; // No program header table
    }

    return ENONE; // ELF header is valid
}

int elf32_loader_process_loaded_elf(elf32_loader_file_t* elf_file) {
    int res = 0;
    elf32_header_t* elf_header = ELF32_LOADER_ELF_HEADER(elf_file);
    res = elf32_loader_validate_elf_header(elf_header);
    if (res != ENONE) {
        goto exit;
    }

    /**
     * At this point in time, the ELF file is only considered as an executable.
     * Linking and Relocatable ELF files are not supported yet.
     */
    // Process program headers and map memory addresses
    res = elf32_loader_process_pheaders(elf_file);
    if (res != ENONE) {
        goto exit;
    }

exit:
    return res;
}

/**
 * Process the loaded ELF data and prepare it for execution.
 * @param elf_file Pointer to the elf_file_t structure containing loaded ELF data.
 * @return ENONE on success, or a negative error code on failure.
 * @note In case memory leaks occur, ensure to call elf32_loader_close to free resources.
 */
int elf32_loader_load(const char* filename, elf32_loader_file_t** out_elf_file) {
    int res = 0;
    // Prepare elf_file structure
    elf32_loader_file_t* elf_file = (elf32_loader_file_t*)kheap_zmalloc(sizeof(elf32_loader_file_t));
    if (!elf_file) {
        res = -ENOMEM;
        goto exit;
    }
    // Open the ELF file from disk
    int fd = file_open(filename, "r");
    if (fd < 0) {
        res = fd; // Propagate error code
        goto exit;
    }
    // Get file status for size
    file_state_t f_stat;
    res = file_stat(fd, &f_stat);
    if (res != ENONE) {
        goto exit;
    }

    // Allocate memory for ELF data
    elf_file->elf_data = kheap_zmalloc(f_stat.file_size);
    if (!elf_file->elf_data) {
        res = -ENOMEM;
        goto exit;
    }
    elf_file->in_memory_size = f_stat.file_size;

    // Read ELF data into memory
    size_t total_read = file_read(elf_file->elf_data, f_stat.file_size, 1, fd);
    if (total_read < 0) {
        res = total_read; // Propagate error code
        if ((size_t)total_read < (size_t)f_stat.file_size) {
            res = -EIO; // Input/output error
        }
        goto exit;
    }

    // Process loaded ELF data (e.g., parse headers)
    res = elf32_loader_process_loaded_elf(elf_file);
    if (res != ENONE) {
        goto exit;
    }

exit:
    // Cleanup before returning
    if (fd >= 0) {
        file_close(fd);
    }
    if (res < 0 && elf_file) {
        elf32_loader_close(elf_file);
        elf_file = NULL;
    }
    // Set output parameter
    if (out_elf_file) {
        *out_elf_file = elf_file;
    }
    return res;
}

/**
 * Close an ELF file and free associated resources.
 * @param elf_file Pointer to the elf32_loader_file_t structure to be closed.
 */
void elf32_loader_close(elf32_loader_file_t* elf_file) {
    if (!elf_file) {
        return;
    }
    // Free the allocated memory for ELF data
    if (elf_file->elf_data) {
        kheap_free(elf_file->elf_data);
        elf_file->elf_data = NULL;
    }
    // Free the elf_file structure itself
    kheap_free(elf_file);
}