#include "file.h"

/**
 * @file file.c
 * @brief File system and file descriptor management
 * @path /home/hayman/Workspace/PeachOS/src/fs/file.c
 * 
 * @details This module defines tables to hold file systems and file descriptors.
 * Each slot in the tables stores a pointer to the respective structure, allowing
 * the contents to be replaced freely. To leverage this design, search operations
 * should return pointers to the slots rather than the structures themselves.
 */

static file_system_t* file_systems[FS_MAX_FILE_SYSTEMS];
static file_descriptor_t* file_descriptors[FS_MAX_FILE_DESCRIPTORS];

/**
 * @brief Find a free slot in the file system table.
 * @return Pointer to the free slot, or NULL if none available.
 */
static file_system_t** file_get_free_file_system_slot() {
    for (int i = 0; i < FS_MAX_FILE_SYSTEMS; i++) {
        if (file_systems[i] == NULL) {
            return &file_systems[i];
        }
    }
    return NULL; // No free slot
}

error_t file_load_file_systems() {
    // Load built-in file systems here (e.g., FAT32, ext4, etc.)
    // For now, we can leave this function empty or add dummy FS.

    // Reset the file system table
    for (int i = 0; i < FS_MAX_FILE_SYSTEMS; i++) {
        file_systems[i] = NULL;
    }

    return ENONE;
}

/**
 * @brief Initialize the file system module.
 * @return 0 on success, negative error code on failure.
 */
error_t file_init() {
    // Initialize file system and descriptor tables

    // Reset file descriptor table
    for (int i = 0; i < FS_MAX_FILE_DESCRIPTORS; i++) {
        file_descriptors[i] = NULL;
    }

    // Load file systems
    if (file_load_file_systems() != ENONE) {
        return -EIO;
    }

    return ENONE;
}

/**
 * @brief Insert a new file system into the file system table.
 * @param fs Pointer to the file system to insert.
 * @return 0 on success, negative error code on failure.
 */
error_t file_insert_file_system(file_system_t* fs) {
    // Insert a new file system into the file_systems table
    file_system_t** slot = file_get_free_file_system_slot();
    if (slot != NULL) {
        *slot = fs;
        return ENONE;
    }
    return -EIO; // No free slot available
}

/**
 * @brief Resolve the appropriate file system for a given disk.
 * @param disk Pointer to the disk to resolve.
 * @return Pointer to the resolved file system, or NULL if none found.
 */
file_system_t* file_system_resolve(disk_t* disk) {
    // Resolve and return the appropriate file system for the given disk
    for (int i = 0; i < FS_MAX_FILE_SYSTEMS; i++) {
        file_system_t* fs = file_systems[i];
        if (fs != NULL && fs->resolve != NULL) {
            if (fs->resolve(disk) == 0) {
                return fs; // Found matching file system
            }
        }
    }
    return NULL; // No matching file system found
}