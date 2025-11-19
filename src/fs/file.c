#include "file.h"
#include "fat/fat16.h"
#include "pparser.h"
#include "utils/string.h"
#include "memory/heap/kheap.h"

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
static file_descriptor_t* file_descriptors[FS_MAX_FILE_DESCRIPTORS]; // 1-based indexing
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

/**
 * @brief Load built-in file systems into the file system table.
 * @return 0 on success, negative error code on failure.
 */
error_t file_load_file_systems() {
    // Load built-in file systems here (e.g., FAT32, ext4, etc.)
    // For now, we can leave this function empty or add dummy FS.

    // Reset the file system table
    for (int i = 0; i < FS_MAX_FILE_SYSTEMS; i++) {
        file_systems[i] = NULL;
    }

    // Load FAT16 file system as an example
    file_system_t* fat16_fs = fat16_init();
    if (fat16_fs == NULL) {
        return -EIO; // Error initializing FAT16
    }
    error_t res = file_insert_file_system(fat16_fs);
    if (res != ENONE) {
        return res; // Propagate error
    }

    return ENONE;
}

file_mode_t file_get_mode_from_string(const char* mode_str) {
    if (strncmp(mode_str, "r", 1) == 0) {
        return FILE_MODE_READ;
    } else if (strncmp(mode_str, "w", 1) == 0) {
        return FILE_MODE_WRITE;
    } else if (strncmp(mode_str, "a", 1) == 0) {
        return FILE_MODE_APPEND;
    } else if (strncmp(mode_str, "r+", 2) == 0) {
        return FILE_MODE_READ | FILE_MODE_WRITE;
    } else if (strncmp(mode_str, "w+", 2) == 0) {
        return FILE_MODE_READ | FILE_MODE_WRITE;
    } else if (strncmp(mode_str, "a+", 2) == 0) {
        return FILE_MODE_READ | FILE_MODE_APPEND;
    }
    return FILE_MODE_INVALID; // Invalid mode
}

/**
 * @brief Create a new file descriptor and add it to the descriptor table.
 * @param out_fd Pointer to store the newly created file descriptor.
 * @return 0 on success, negative error code on failure.
 */
int file_new_descriptor(file_descriptor_t** out_fd) {
    for (int i = 0; i < FS_MAX_FILE_DESCRIPTORS; i++) {
        if (file_descriptors[i] == NULL) {
            // Allocate and initialize a new file descriptor
            file_descriptor_t* fd = kheap_zmalloc(sizeof(file_descriptor_t));
            if (!fd) {
                return -ENOMEM; // Memory allocation error
            }
            fd->id = i + 1; // File descriptor IDs start from 1.
            file_descriptors[i] = fd;
            *out_fd = fd;
            return ENONE; // Success
        }
    }
    return -EBUSY; // No free file descriptor slots
}

/**
 * @brief Retrieve a file descriptor by its ID.
 * @param fd_id The ID of the file descriptor to retrieve.
 * @return Pointer to the file descriptor, or NULL if not found.
 */
file_descriptor_t* file_get_descriptor_by_id(uint32_t fd_id) {
    if (fd_id == 0 || fd_id > FS_MAX_FILE_DESCRIPTORS) {
        return NULL; // Invalid file descriptor ID
    }
    return file_descriptors[fd_id - 1];
}

/**********************/
/* Exported Functions */
/**********************/

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

/**
 * @brief Open a file given its path and mode.
 * @param path The file path to open.
 * @param mode The mode string (e.g., "r", "w", "a").
 * @return File descriptor ID on success, negative error code on failure.
 */
int file_open(const char* path, const char* mode) {
    int res = 0;
    
    // Parse the path to get the disk and path parts
    path_root_t* parsed_path = path_parse(path);
    if (!parsed_path) {
        return -EINVAL; // Invalid path
    }

    // Get the disk by drive number
    disk_t* disk = disk_get_by_uid(parsed_path->drive_no);
    if (!disk || !disk->fs) {
        res = -ENOTFOUND; // Disk not found
        goto exit;
    }

    // Parse the mode string
    file_mode_t file_mode = file_get_mode_from_string(mode);
    if (file_mode == FILE_MODE_INVALID) {
        res = -EINVAL; // Invalid mode
        goto exit;
    }

    // Open the file using the file system's open function
    void* file_handle = disk->fs->open(disk, parsed_path->first, file_mode);
    if (IS_ERROR(file_handle)) {
        res = -EIO; // Error opening file
        goto exit;
    }

    // Get a free file descriptor slot
    file_descriptor_t* fd;
    res = file_new_descriptor(&fd);
    if (res != ENONE) {
        // No free file descriptor slots
        goto exit;
    }
    fd->fs = disk->fs;
    fd->disk = disk;
    fd->fs_private_data = file_handle;
    res = fd->id; // Return the file descriptor ID

exit:
    if (parsed_path) {
        path_free(parsed_path);
    }
    return res;
}

size_t file_read(void* buffer, size_t size, size_t nmemb, int fd_id) {
    file_descriptor_t* fd = file_get_descriptor_by_id(fd_id);
    if (!fd || !fd->fs || !fd->fs->read) {
        return (size_t)-EBADF; // Bad file descriptor
    }
    return fd->fs->read(fd, size, nmemb, buffer);
}