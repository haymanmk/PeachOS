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
    fs_data->root_directory.end_pos = root_dir_position_sectors + root_dir_size_sectors -1;

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
 * @brief Get the full file name (with extension) from a FAT16 directory entry.
 * @param entry Pointer to the FAT16 directory entry.
 * @param out_name Buffer to store the resulting file name (should be at least 13 bytes).
 */
void fat16_get_full_name_from_entry(fat_directory_entry_t* entry, char* out_name) {
    // Copy name part and trim spaces
    int i, j;
    for (i = 0; i < 8 && entry->name[i] != ' '; i++) {
        out_name[i] = entry->name[i];
    }

    // Check for extension
    if (entry->ext[0] != ' ') {
        out_name[i++] = '.'; // Add dot before extension
        for (j = 0; j < 3 && entry->ext[j] != ' '; j++) {
            out_name[i++] = entry->ext[j];
        }
    }
    out_name[i] = '\0'; // Null-terminate the string
}

/**
 * @brief Get the full file name (with extension) from a FAT16 directory entry.
 * @param entry Pointer to the FAT16 directory entry.
 * @param out_name Buffer to store the resulting file name (should be at least 13 bytes).
 * @return 1 on success, 0 if not a regular file, negative error code on failure.
 */
int fat16_get_full_file_name(fat_directory_entry_t* entry, char* out_name) {
    // Check for deleted or empty entry
    if (entry->name[0] == 0x00 || entry->name[0] == 0xE5) {
        return -EINVAL; // Invalid entry
    }

    // If it is a long file name entry, return 0
    if ((entry->attributes & FAT_FILE_ATTR_LONG_NAME) == FAT_FILE_ATTR_LONG_NAME) {
        return 0; // Not a regular file
    }

    // If it is a directory, return 0
    if ((entry->attributes & FAT_FILE_ATTR_DIRECTORY) == FAT_FILE_ATTR_DIRECTORY) {
        return 0; // Not a regular file
    }

    // Copy name part and trim spaces
    fat16_get_full_name_from_entry(entry, out_name);

    return 1; // Success
}

/**
 * @brief Search for a file in a FAT16 directory by name.
 * @param directory Pointer to the FAT16 directory.
 * @param name Name of the file to search for.
 * @return Pointer to the directory entry if found, NULL otherwise.
 */
fat_directory_entry_t* fat16_search_file(fat_directory_t* directory, const char* name) {
    for (uint32_t i = 0; i < directory->in_use_entry_count; i++) {
        fat_directory_entry_t* entry = &directory->entries[i];
        // Get the full file name with extension. Get 1 on success, 0 if not a file.
        char filename[13];
        if ((fat16_get_full_file_name(entry, filename)) != 1) {
            continue; // Not a regular file
        }
        if (strcmp_ignore_case(filename, name) == 0) {
            return entry; // Found the file
        }
    }
    return NULL; // File not found
}

/**
 * @brief Get the directory name from a FAT16 directory entry.
 * @param entry Pointer to the FAT16 directory entry.
 * @param out_name Buffer to store the resulting directory name (should be at least 13 bytes).
 * @return 1 on success, 0 if not a directory, negative error code on failure.
 */
int fat16_get_directory_name_from_entry(fat_directory_entry_t* entry, char* out_name) {
    // Check for deleted or empty entry
    if (entry->name[0] == 0x00 || entry->name[0] == 0xE5) {
        return -EINVAL; // Invalid entry
    }

    // If it is a long file name entry, return 0
    if ((entry->attributes & FAT_FILE_ATTR_LONG_NAME) == FAT_FILE_ATTR_LONG_NAME) {
        return 0; // Not a regular directory
    }

    // If it is not a directory, return 0
    if ((entry->attributes & FAT_FILE_ATTR_DIRECTORY) != FAT_FILE_ATTR_DIRECTORY) {
        return 0; // Not a directory
    }

    // Copy name part and trim spaces
    fat16_get_full_name_from_entry(entry, out_name);

    return 1; // Success
}

/**
 * @brief Calculate the starting sector of a given cluster number in FAT16.
 * @param disk Pointer to the disk.
 * @param cluster_number Cluster number to calculate the starting sector for.
 * @return Starting sector number of the cluster, or negative error code on failure.
 */
int fat16_calculate_cluster_start_sector(disk_t* disk, uint16_t cluster_number) {
    if (cluster_number < 2) {
        return -EINVAL; // Invalid cluster number
    }

    fat_fs_private_data_t* fs_data = (fat_fs_private_data_t*)disk->private_data;
    fat_common_header_t* primary_header = &fs_data->header.common;
    fat_directory_t* root_directory = &fs_data->root_directory;
    uint32_t first_data_sector = root_directory->end_pos + 1;

    uint32_t sectors_per_cluster = primary_header->sectors_per_cluster;
    uint32_t cluster_offset = (cluster_number - 2) * sectors_per_cluster; // Subtract 2 for FAT16 cluster numbering,
                                                                          // because the root directory of FAT32 starts at cluster 2.
    return (first_data_sector + cluster_offset);
}

/**
 * @brief Get the subdirectory information from a FAT16 directory entry.
 * @param disk Pointer to the disk.
 * @param entry Pointer to the FAT16 directory entry representing the subdirectory.
 * @param out_directory Pointer to store the resulting FAT16 directory information. Important: The caller is responsible for allocating memory for out_directory and its entries.
 * @return 0 on success, negative error code otherwise.
 */
int fat16_get_subdirectory(disk_t* disk, fat_directory_entry_t* entry, fat_directory_t** out_directory) {
    // Get the volume ID of subdirectory from current_directory, and then copy the directory entries into out_directory
    // Convert private data
    fat_fs_private_data_t* fs_data = (fat_fs_private_data_t*)disk->private_data;
    fat_common_header_t* primary_header = &fs_data->header.common;
    uint16_t root_entry_count = primary_header->root_entry_count;

    // Read the volume ID of the subdirectory from the current_directory
    // Note: In FAT16, the starting cluster number is stored in first_cluster_low
    uint16_t starting_cluster = entry->first_cluster_low;
    if (starting_cluster == 0) {
        return -EINVAL; // Invalid starting cluster
    }
    // Calculate the starting position (sector) of the subdirectory
    int start_sector = fat16_calculate_cluster_start_sector(disk, starting_cluster);
    if (start_sector < 0) {
        return start_sector; // Propagate error
    }
    // Read the subdirectory entries from disk
    disk_streamer_t* dir_streamer = fs_data->directory_streamer;
    if (disk_streamer_seek(dir_streamer, start_sector * disk->sector_size) < 0) {
        return -EIO; // I/O error
    }
    // Allocate memory for subdirectory entries
    size_t entry_size = root_entry_count * sizeof(fat_directory_entry_t);
    if (disk_streamer_read(dir_streamer, entry_size, (*out_directory)->entries) < 0) {
        return -EIO; // I/O error
    }

    // Count in-use entries
    uint32_t in_use_count = fat16_count_in_use_entries((*out_directory)->entries, root_entry_count);

    // Populate the out_directory structure
    (*out_directory)->in_use_entry_count = in_use_count;
    (*out_directory)->start_pos = start_sector;
    (*out_directory)->end_pos = start_sector + (entry_size / disk->sector_size) - 1;

    return 0;
}

/**
 * @brief Search for a subdirectory in a FAT16 directory by name.
 * @param disk Pointer to the disk.
 * @param current_directory Pointer to the current FAT16 directory.
 * @param name Name of the subdirectory to search for.
 * @param out_directory Pointer to store the resulting FAT16 subdirectory information. Important: The caller is responsible for allocating memory for out_directory and its entries.
 * @return 0 on success, negative error code otherwise.
 */
int fat16_search_directory(disk_t* disk, fat_directory_t* current_directory, const char* name, fat_directory_t** out_directory) {
    int res = 0;
    for (uint32_t i = 0; i < current_directory->in_use_entry_count; i++) {
        fat_directory_entry_t* entry = &current_directory->entries[i];
        // Check if the entry is a directory
        if ((entry->attributes & FAT_FILE_ATTR_DIRECTORY) != FAT_FILE_ATTR_DIRECTORY) {
            continue; // Not a directory
        }
        // Get the full directory name
        char dirname[13];
        if (fat16_get_directory_name_from_entry(entry, dirname) != 1) {
            continue; // Not a valid directory
        }
        if (strncmp_ignore_case(dirname, name, 13) == 0) {
            // Get the sub-directory information
            res = fat16_get_subdirectory(disk, entry, out_directory);
            if (res < 0) {
                return res;
            }
            return ENONE; // Found the directory
        }
    }

    return -ENOTFOUND; // Directory not found
}

/**
 * @brief Allocate a temporary FAT16 directory structure.
 * @param entry_count Number of entries to allocate space for.
 * @return Pointer to the allocated FAT16 directory, or NULL on memory allocation error.
 */
fat_directory_t* fat16_allocate_temp_directory(uint32_t entry_count) {
    fat_directory_t* temp_dir = (fat_directory_t*)kheap_zmalloc(sizeof(fat_directory_t));
    if (!temp_dir) {
        return NULL; // Memory allocation error
    }
    fat_directory_entry_t* entries = (fat_directory_entry_t*)kheap_zmalloc(
        entry_count * sizeof(fat_directory_entry_t));
    if (!entries) {
        kheap_free(temp_dir);
        return NULL; // Memory allocation error
    }
    temp_dir->entries = entries;
    return temp_dir;
}

/**
 * @brief Free a temporary FAT16 directory structure.
 * @param temp_dir Pointer to the FAT16 directory to free.
 */
void fat16_free_temp_directory(fat_directory_t* temp_dir) {
    if (temp_dir) {
        if (temp_dir->entries) {
            kheap_free(temp_dir->entries);
        }
        kheap_free(temp_dir);
    }
}

/**
 * @brief Get the FAT16 directory entry for a file given its path.
 * @param disk Pointer to the disk.
 * @param path_part Pointer to the path part structure representing the file path.
 * @return Pointer to the FAT16 directory entry if found, NULL otherwise.
 */
fat_directory_entry_t* fat16_get_file_entry_from_path(disk_t* disk, path_part_t* path_part) {
    fat_fs_private_data_t* fs_data = (fat_fs_private_data_t*)disk->private_data;
    fat_directory_t* current_directory = &fs_data->root_directory;
    fat_directory_entry_t* entry = NULL;
    fat_directory_t* temp_dir;

    // Traverse the path parts to find the file entry
    while (path_part) {
        // if the current path part is the last one, we are looking for a file
        if (path_part->next == NULL) {
            entry = fat16_search_file(current_directory, path_part->name);
            break;
        }

        // else we are looking for a directory
        temp_dir = fat16_allocate_temp_directory(fs_data->header.common.root_entry_count);
        if (!temp_dir) {
            return NULL; // Memory allocation error
        }
        int res = fat16_search_directory(disk, current_directory, path_part->name, &temp_dir);
        if (res < 0) {
            goto exit;
        }
        // Free the previous directory if it was a temp one
        if (current_directory != &fs_data->root_directory) {
            fat16_free_temp_directory(current_directory);
        }
        current_directory = temp_dir;
        path_part = path_part->next;
    }

exit:
    if (temp_dir) {
        fat16_free_temp_directory(temp_dir);
    }
    return entry; // Should not reach here
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
    int res = 0;

    // Allocate and initialize a fat_file_directory_representation_t structure
    fat_file_directory_representation_t* file_rep = kheap_zmalloc(sizeof(fat_file_directory_representation_t));
    if (!file_rep) {
        return NULL; // Memory allocation error
    }

    // Get file entry from root directory
    file_rep->type = FAT_DIRECTORY_ENTRY_TYPE_FILE;
    file_rep->sfn_entry = fat16_get_file_entry_from_path(disk, path_part);
    if (!file_rep->sfn_entry) {
        res = -ENOTFOUND; // File not found
        goto exit;
    }

exit:
    if (res < 0) {
        // Cleanup on failure
        if (file_rep) {
            kheap_free(file_rep);
            file_rep = NULL;
        }
        // Parse error code
        return ERROR_VOID(res); // Return error as void pointer.
                                // Caller should check using IS_ERROR macro or ERROR_CODE macro.
    }
    return file_rep;
}

/**
 * @brief Initialize the FAT16 file system.
 * @return Pointer to the initialized FAT16 file system structure.
 */
file_system_t* fat16_init() {
    return &fat16_fs;
}