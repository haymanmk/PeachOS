#include "fat16.h"
#include "utils/string.h"
#include "disk/disk.h"

/* Function prototypes */
int fat16_resolve(disk_t* disk);
void* fat16_open(disk_t* disk, path_part_t* path_part, file_mode_t mode);

static file_system_t fat16_fs = {
    .name = "FAT16",
    .resolve = fat16_resolve,
    .open = fat16_open
};

/**
 * @brief Resolve if the disk contains a FAT16 file system.
 * @param disk Pointer to the disk to check.
 * @return 0 if the disk contains FAT16, negative error code otherwise.
 */
int fat16_resolve(disk_t* disk) {
    // Read the boot sector and check for FAT16 signature
    uint8_t buffer[512];
    if (disk_read_lba(disk, 0, 1, buffer) != 0) {
        return -EIO; // I/O error
    }
    // Check for FAT16 signature (0x29 at offset 38)
    if (buffer[38] == 0x29) {
        return 0; // FAT16 detected
    }
    return -ENOTFOUND; // Not a FAT16 file system
}

/**
 * @brief Open a file on a FAT16 file system.
 * @param disk Pointer to the disk where the file resides.
 * @param path_part Pointer to the path part structure representing the file path.
 * @param mode The mode in which to open the file.
 * @return Pointer to the file descriptor, or NULL on failure.
 */
void* fat16_open(disk_t* disk, path_part_t* path_part, file_mode_t mode) {
    // Implementation of opening a file on FAT16 goes here
    // For now, return NULL to indicate not implemented
    return NULL;
}

/**
 * @brief Initialize the FAT16 file system.
 * @return Pointer to the initialized FAT16 file system structure.
 */
file_system_t* fat16_init() {
    return &fat16_fs;
}