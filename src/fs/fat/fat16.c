#include "fat_common.h"
#include "fat16.h"
#include "utils/string.h"
#include "disk/disk.h"
#include "memory/heap/kheap.h"

/* Function prototypes */
int fat16_resolve(disk_t* disk);
void* fat16_open(disk_t* disk, path_part_t* path_part, file_mode_t mode);

static file_system_t fat16_fs = {
    .name = "FAT16",
    .resolve = fat16_resolve,
    .open = fat16_open
};

/**
 * @brief Count the number of in-use entries in a FAT16 directory.
 * @param entries Pointer to the array of directory entries.
 * @param total_entries Total number of entries in the directory.
 * @return Number of in-use entries.
 */
uint32_t fat16_count_in_use_entries(fat_directory_entry_t* entries, uint32_t total_entries) {
    uint32_t count = 0;
    for (uint32_t i = 0; i < total_entries; i++) {
        if (entries[i].name[0] != 0x00 && entries[i].name[0] != 0xE5) {
            count++;
        }
    }
    return count;
}

/**
 * @brief Get the root directory information for a FAT16 file system.
 * @param disk Pointer to the disk.
 * @param fs_data Pointer to the FAT file system private data.
 * @return 0 on success, negative error code otherwise.
 */
int fat16_get_root_directory(disk_t* disk, fat_fs_private_data_t* fs_data) {
    int res = 0;
    fat_common_header_t* primary_header = &fs_data->header.common;
    uint32_t root_dir_position_sectors = primary_header->reserved_sector_count +
                                        (primary_header->num_fats * primary_header->fat_size_16);
    uint32_t root_dir_size_bytes = primary_header->root_entry_count * sizeof(fat_directory_entry_t);
    uint32_t root_dir_size_sectors = (root_dir_size_bytes + disk->sector_size - 1) / disk->sector_size;

    // Allocate memory for root directory entries
    fat_directory_entry_t* entries = (fat_directory_entry_t*)kheap_zmalloc(root_dir_size_bytes);
    if (!entries) {
        res = -ENOMEM; // Memory allocation error
        goto exit;
    }
    // Read root directory entries from disk
    disk_streamer_t* dir_streamer = fs_data->directory_streamer;
    if (disk_streamer_seek(dir_streamer, root_dir_position_sectors * disk->sector_size) < 0) {
        res = -EIO; // I/O error
        goto exit;
    }
    if (disk_streamer_read(dir_streamer, root_dir_size_bytes, entries) < 0) {
        res = -EIO; // I/O error
        goto exit;
    }

    // Count in-use entries
    uint32_t in_use_count = fat16_count_in_use_entries(entries, primary_header->root_entry_count);

    fs_data->root_directory.entries = entries;
    fs_data->root_directory.in_use_entry_count = in_use_count;
    fs_data->root_directory.start_pos = root_dir_position_sectors;
    fs_data->root_directory.end_pos = root_dir_position_sectors + root_dir_size_sectors - 1;

exit:
    if (res < 0) {
        // Cleanup on failure
        if (entries) {
            kheap_free(entries);
        }
    }

    return res;
}

/**
 * @brief Resolve if the disk contains a FAT16 file system.
 *        Note: This function allocates memory for FAT file system private data
 *              and assigns it to disk->private_data on success.
 * @param disk Pointer to the disk to check.
 * @return 0 if the disk contains FAT16, negative error code otherwise.
 */
int fat16_resolve(disk_t* disk) {
    int res = 0;
    disk_streamer_t* stream = NULL;
    fat_fs_private_data_t* fs_data = kheap_zmalloc(sizeof(fat_fs_private_data_t));
    if (!fs_data) {
        return -ENOMEM; // Memory allocation error
    }
    // Initialize streamers
    fs_data->cluster_streamer = disk_streamer_create(disk->uid);
    fs_data->fat_read_streamer = disk_streamer_create(disk->uid);
    fs_data->directory_streamer = disk_streamer_create(disk->uid);
    if (!fs_data->cluster_streamer || !fs_data->fat_read_streamer || !fs_data->directory_streamer) {
        res = -ENOMEM; // Memory allocation error
        goto exit;
    }

    // Read the FAT header
    if (!(stream = disk_streamer_create(disk->uid))) {
        res = -EIO; // I/O error
        goto exit;
    }
    if ((res = disk_streamer_seek(stream, 0)) < 0) {
        goto exit;
    }
    if ((res = disk_streamer_read(stream, sizeof(fs_data->header), &fs_data->header)) < 0) {
        goto exit;
    }

    // Check for FAT16 signature (0x29 at offset 38)
    if (fs_data->header.extended.fat16.boot_signature != 0x29) {
        res = -ENOTFOUND; // Not a FAT16 file system
        goto exit;
    }

    // Get root directory information
    if ((res = fat16_get_root_directory(disk, fs_data)) < 0) {
        goto exit;
    }

    // If we reach here, it's a FAT16 file system
    disk->private_data = fs_data;
    disk->fs = &fat16_fs;

exit:
    if (stream)
    {
        disk_streamer_destroy(stream);
    }

    if (res < 0) {
        // Cleanup on failure
        if (fs_data->cluster_streamer) {
            disk_streamer_destroy(fs_data->cluster_streamer);
        }
        if (fs_data->fat_read_streamer) {
            disk_streamer_destroy(fs_data->fat_read_streamer);
        }
        if (fs_data->directory_streamer) {
            disk_streamer_destroy(fs_data->directory_streamer);
        }
        kheap_free(fs_data);
        disk->private_data = NULL;
    }

    return res;
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