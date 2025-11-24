#include "fat_common.h"
#include "fat16.h"
#include "utils/string.h"
#include "disk/disk.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"

/* Function prototypes */
int fat16_resolve(disk_t* disk);
void* fat16_open(disk_t* disk, path_part_t* path_part, file_mode_t mode);
size_t fat16_read(file_descriptor_t* fd, size_t size, size_t nmemb, void* buffer);
int fat16_seek(file_descriptor_t* fd, int32_t offset, file_seek_mode_t whence);
int fat16_stat(file_descriptor_t* fd, file_state_t* out_state);
int fat16_close(file_descriptor_t* fd);

static file_system_t fat16_fs = {
    .name = "FAT16",
    .resolve = fat16_resolve,
    .open = fat16_open,
    .read = fat16_read,
    .seek = fat16_seek,
    .stat = fat16_stat,
    .close = fat16_close
};

/**
 * @brief Count the number of in-use entries in a FAT16 directory.
 * @param disk Pointer to the disk.
 * @param directory_start_sector Starting sector of the directory.
 * @return Number of in-use entries, or negative error code on failure.
 */
int fat16_count_in_use_entries(disk_t* disk, uint32_t directory_start_sector) {
    int res = 0;
    fat_directory_entry_t entry;
    uint32_t total_entries = 0;
    fat_fs_private_data_t* fs_data = (fat_fs_private_data_t*)disk->private_data;
    disk_streamer_t *dir_streamer = fs_data->directory_streamer;
    uint32_t dir_start_pos = directory_start_sector * disk->sector_size;
    // Seek to the start of the directory
    if (disk_streamer_seek(dir_streamer, dir_start_pos) < 0) {
        return -EIO; // I/O error
    }
    // Read all directory entries
    while (1) {
        // Note: The disk_streamer_read will increment the position internally, i.e. dir_streamer->pos
        if (disk_streamer_read(dir_streamer, sizeof(fat_directory_entry_t), &entry) < 0) {
            res = -EIO; // I/O error
            break;
        }
        // Check for end of directory
        if (entry.name[0] == 0x00) {
            break; // End of directory
        }
        // Check for deleted entry
        if (entry.name[0] == 0xE5) {
            continue; // Deleted entry, skip
        }
        total_entries++;
    }
    res = total_entries;
    return res;
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
    int in_use_count = fat16_count_in_use_entries(disk, root_dir_position_sectors);
    if (in_use_count < 0) {
        res = in_use_count; // Propagate error
        goto exit;
    }

    fs_data->root_directory.entries = entries;
    fs_data->root_directory.in_use_entry_count = (uint32_t)in_use_count;
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
 * @brief Get the full file/directory name (with extension) from a FAT16 directory entry.
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
 * @brief Read a FAT16 entry from the FAT table.
 * @param disk Pointer to the disk.
 * @param cluster_number Cluster number to read the FAT entry for.
 * @return FAT16 entry value, or 0 on failure.
 */
uint16_t fat16_read_entry_from_fat_table(disk_t* disk, uint16_t cluster_number) {
    fat_fs_private_data_t* fs_data = (fat_fs_private_data_t*)disk->private_data;
    disk_streamer_t* fat_streamer = fs_data->fat_read_streamer;
    uint32_t fat_offset = cluster_number * FAT16_FAT_ENTRY_SIZE; // Each FAT16 entry is 2 bytes
    uint32_t fat_start_pos = fs_data->header.common.reserved_sector_count * disk->sector_size;
    uint32_t entry_pos = fat_start_pos + fat_offset;

    // Seek to the FAT entry position
    if (disk_streamer_seek(fat_streamer, entry_pos) < 0) {
        return 0; // I/O error
    }
    uint16_t fat_entry;
    // Read the FAT entry
    if (disk_streamer_read(fat_streamer, sizeof(uint16_t), &fat_entry) < 0) {
        return 0; // I/O error
    }
    return fat_entry;
}

/**
 * @brief Get the cluster number corresponding to a given offset from a starting cluster in FAT16.
 * @param disk Pointer to the disk.
 * @param start_cluster Starting cluster number.
 * @param offset Offset in bytes from the start_cluster.
 * @return Cluster number at the given offset, or negative error code on failure.
 */
int fat16_get_cluster_from_offset(disk_t* disk, uint16_t start_cluster, uint32_t offset) {
    int res = 0;
    fat_fs_private_data_t* fs_data = (fat_fs_private_data_t*)disk->private_data;
    uint32_t cluster_size_bytes = fs_data->header.common.sectors_per_cluster * disk->sector_size;
    uint16_t current_cluster = start_cluster;
    uint32_t clusters_to_advance = offset / cluster_size_bytes; // Number of clusters to advance

    // Go through cluster chain to find the target cluster
    for (uint32_t i = 0; i < clusters_to_advance; i++) {
        uint16_t fat_entry = fat16_read_entry_from_fat_table(disk, current_cluster); // Read FAT entry which gives the next cluster in the chain
        if (fat_entry >= 0xFFF8) {
            // End of cluster chain
            res = -ENODATA; // No more data
            goto exit;
        }
        if (fat_entry == 0x0000 || fat_entry == 0xFFF7) {
            // Bad cluster, free cluster, or read entry failed
            res = -EIO; // I/O error
            goto exit;
        }
        current_cluster = fat_entry;
    }
    res = current_cluster;
exit:
    return res;
}

/**
 * @brief Read bytes from clusters starting from a given cluster in FAT16.
 * @param disk Pointer to the disk.
 * @param start_cluster Starting cluster number.
 * @param offset_from_start Offset from the start_cluster in bytes.
 * @param total_bytes Total number of bytes to read.
 * @param buffer Buffer to store the read bytes.
 */
int fat16_read_bytes_in_cluster_chain(disk_t* disk, uint16_t start_cluster, uint32_t offset_from_start, uint32_t total_bytes, void* buffer) {
    int res = 0;
    uint32_t offset = offset_from_start; // in bytes
    fat_fs_private_data_t* fs_data = (fat_fs_private_data_t*)disk->private_data;
    uint32_t cluster_size_bytes = fs_data->header.common.sectors_per_cluster * disk->sector_size;
    disk_streamer_t* cluster_streamer = fs_data->cluster_streamer;
    int current_cluster = start_cluster;
    uint32_t total_to_read;

    while (total_bytes > 0) {
        // Get the starting cluster to read according to the offset and start_cluster
        current_cluster = fat16_get_cluster_from_offset(disk, current_cluster, offset);
        if (current_cluster < 0) {
            res = -EIO; // I/O error
            goto exit;
        }
        uint32_t starting_sector = fat16_calculate_cluster_start_sector(disk, (uint16_t)current_cluster);
        offset %= cluster_size_bytes; // ensure offset is within the cluster size
        uint32_t starting_pos = (starting_sector * disk->sector_size) + offset;
        res = disk_streamer_seek(cluster_streamer, starting_pos);
        if (res < 0) {
            res = -EIO; // I/O error
            goto exit;
        }
        // Calculate how many bytes to read in this iteration
        total_to_read = cluster_size_bytes - offset;
        if (total_to_read > total_bytes) {
            total_to_read = total_bytes;
        }
        res = disk_streamer_read(cluster_streamer, total_to_read, buffer);
        if (res < 0) {
            res = -EIO; // I/O error
            goto exit;
        }
        buffer += total_to_read;
        total_bytes -= total_to_read;
        offset += total_to_read;
    }

exit:
    return res;
}

fat_directory_t* fat16_load_directory(disk_t* disk, fat_directory_entry_t* entry) {
    // Return if the entry is not a directory
    if (!(entry->attributes & FAT_FILE_ATTR_DIRECTORY)) {
        return NULL; // Not a directory
    }
    fat_directory_t* directory = (fat_directory_t*)kheap_zmalloc(sizeof(fat_directory_t));
    if (!directory) {
        return NULL; // Memory allocation error
    }

    // Prepare entries buffer for the directory
    // Find the first cluster of the directory
    uint32_t first_cluster = entry->first_cluster_low;
    if (first_cluster < 2) {
        // Invalid cluster number for a directory
        goto failed;
    }
    // Calculate the starting sector of the directory
    int start_sector = fat16_calculate_cluster_start_sector(disk, first_cluster);
    if (start_sector < 0) {
        goto failed; // Error calculating start sector
    }
    int total_entries = fat16_count_in_use_entries(disk, start_sector);
    if (total_entries < 0) {
        goto failed; // Error counting entries
    }

    // Allocate memory for directory entries
    uint32_t entries_size = total_entries * sizeof(fat_directory_entry_t);
    directory->entries = (fat_directory_entry_t*)kheap_zmalloc(entries_size);
    if (!directory->entries) {
        goto failed; // Memory allocation error
    }
    // Read directory entries from disk
    if (fat16_read_bytes_in_cluster_chain(disk, first_cluster, 0x00, entries_size, directory->entries) < 0) {
        goto failed; // I/O error
    }

    // Populate the rest of directory attributes
    directory->in_use_entry_count = (uint32_t)total_entries;
    directory->start_pos = (uint32_t)start_sector;

    return directory;
failed:
    if (directory) {
        if (directory->entries) {
            kheap_free(directory->entries);
        }
        kheap_free(directory);
    }
    return NULL; // Memory allocation error
}

fat_directory_entry_t* fat16_clone_directory_entry(fat_directory_entry_t* entry) {
    fat_directory_entry_t* cloned_entry = (fat_directory_entry_t*)kheap_zmalloc(sizeof(fat_directory_entry_t));
    if (!cloned_entry) {
        return NULL; // Memory allocation error
    }
    memcpy(cloned_entry, entry, sizeof(fat_directory_entry_t));
    return cloned_entry;
}

fat_file_directory_representation_t* fat16_create_file_directory_representation(disk_t* disk, fat_directory_entry_t* entry) {
    fat_file_directory_representation_t* representation = (fat_file_directory_representation_t*)kheap_zmalloc(sizeof(fat_file_directory_representation_t));
    if (!representation) {
        return NULL; // Memory allocation error
    }

    // Determine the type of the entry
    if (entry->attributes & FAT_FILE_ATTR_DIRECTORY) {
        // Load a directory structure from the volume which might be fragmented across clusters
        fat_directory_t* directory = fat16_load_directory(disk, entry);
        if (!directory) {
            goto failed; // Memory allocation error
        }
        representation->directory = directory;
        representation->type = FAT_DIRECTORY_ENTRY_TYPE_DIRECTORY;
    } else {
        // Clone the directory entry in case the original gets freed
        fat_directory_entry_t* cloned_entry = fat16_clone_directory_entry(entry);
        if (!cloned_entry) {
            goto failed; // Memory allocation error
        }
        representation->sfn_entry = cloned_entry;
        representation->type = FAT_DIRECTORY_ENTRY_TYPE_FILE;
    }
    representation->current_pos = 0; // Initialize current position to 0

    return representation;
failed:
    if (representation) {
        kheap_free(representation);
    }
    return NULL; // Memory allocation error
}

/**
 * @brief Free a FAT16 file/directory representation structure.
 * @param representation Pointer to the FAT16 file/directory representation to free.
 */
void fat16_free_file_directory_representation(fat_file_directory_representation_t* representation) {
    if (representation) {
        // Free both directory and file entry if they exist
        if (representation->type == FAT_DIRECTORY_ENTRY_TYPE_DIRECTORY && representation->directory) {
            // Free directory entries
            if (representation->directory->entries) {
                kheap_free(representation->directory->entries);
            }
            kheap_free(representation->directory);
        } else if (representation->sfn_entry) {
            kheap_free(representation->sfn_entry);
        }
        kheap_free(representation);
    }
}

/**
 * @brief Search for a file in a FAT16 directory by name.
 * @param directory Pointer to the FAT16 directory.
 * @param name Name of the file to search for.
 * @return Pointer to the directory entry if found, NULL otherwise.
 */
fat_file_directory_representation_t* fat16_search_file(disk_t* disk, fat_directory_t* directory, const char* name) {
    fat_file_directory_representation_t* result = NULL;
    for (uint32_t i = 0; i < directory->in_use_entry_count; i++) {
        fat_directory_entry_t* entry = &directory->entries[i];
        // Get the full file name with extension. Get 1 on success, 0 if not a file.
        char filename[13];
        if ((fat16_get_full_file_name(entry, filename)) != 1) {
            continue; // Not a regular file
        }
        if (strcmp_ignore_case(filename, name) == 0) {
            // Found the file, create a representation
            result = fat16_create_file_directory_representation(disk, entry);
        }
    }
    return result; // File not found
}

/**
 * @brief Get the FAT16 directory entry for a file given its path.
 * @param disk Pointer to the disk.
 * @param path_part Pointer to the path part structure representing the file path.
 * @return Pointer to the FAT16 directory entry if found, NULL otherwise.
 */
fat_file_directory_representation_t* fat16_get_file_entry_from_path(disk_t* disk, path_part_t* path_part) {
    int res = 0;
    fat_fs_private_data_t* fs_data = (fat_fs_private_data_t*)disk->private_data;
    // Start from searching for the fist part of the path in the root directory.
    // Note: Item will be either a file or a directory representation.
    fat_file_directory_representation_t* current_item = fat16_search_file(disk, &fs_data->root_directory, path_part->name);
    if (!current_item) {
        return NULL; // File not found in root directory
    }

    // Traverse the path parts to find the file entry in directories
    path_part_t* next_part = path_part->next;
    while (next_part) {
        // if current item(i.e. fat_file_directory_representation_t) is not a directory, return NULL
        if (current_item->type != FAT_DIRECTORY_ENTRY_TYPE_DIRECTORY) {
            // Cleanup and return NULL
            res = -ENOTDIR; // Not a directory
            goto exit;
        }

        // else we look for the next part in the current directory
        fat_file_directory_representation_t* temp_item = fat16_search_file(disk, current_item->directory, next_part->name);
        if (!temp_item) {
            res = -ENOTFOUND; // File not found
            goto exit;
        }
        // Free the previous item if it was a temp one
        fat16_free_file_directory_representation(current_item);
        current_item = temp_item;
        next_part = next_part->next;
    }

exit:
    if (res < 0) {
        // Cleanup on failure
        fat16_free_file_directory_representation(current_item);
        return NULL;
    }
    return current_item; // Should not reach here
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

    disk->private_data = fs_data;
    // Get root directory information
    if ((res = fat16_get_root_directory(disk, fs_data)) < 0) {
        goto exit;
    }

    // If we reach here, it's a FAT16 file system
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
    fat_file_directory_representation_t* file_rep = fat16_get_file_entry_from_path(disk, path_part);
    if (!file_rep) {
        return (void*)-ENOTFOUND; // File not found
    }
    return (void*)file_rep; // Return the file representation as the file handle
}

/**
 * @brief Read data from a FAT16 file.
 * @param fd Pointer to the file descriptor.
 * @param size Size of each element to read. (in bytes)
 * @param nmemb Number of elements to read.
 * @param buffer Buffer to store the read data.
 * @return Number of elements read on success, negative error code on failure.
 */
size_t fat16_read(file_descriptor_t* fd, size_t size, size_t nmemb, void* buffer) {
    if (!fd || !fd->fs) {
        return (size_t)-EBADF; // Bad file descriptor
    }
    disk_t* disk = fd->disk;
    fat_file_directory_representation_t* file_rep = (fat_file_directory_representation_t*)fd->fs_private_data;
    if (!file_rep || file_rep->type != FAT_DIRECTORY_ENTRY_TYPE_FILE) {
        return (size_t)-EBADF; // Bad file descriptor
    }
    fat_directory_entry_t* entry = file_rep->sfn_entry;
    uint32_t offset_from_start = file_rep->current_pos;
    for (size_t i = 0; i < nmemb; i++) {
        // Read size bytes from cluster chain
        int res = fat16_read_bytes_in_cluster_chain(disk, entry->first_cluster_low, offset_from_start, size, (uint8_t*)buffer + i * size);
        if (res < 0) {
            return (size_t)-EIO; // I/O error
        }
        offset_from_start += size;
    }
    // Update current position
    file_rep->current_pos += (uint32_t)(size * nmemb);
    return nmemb;
}

/**
 * @brief Seek to a specific position in a FAT16 file.
 * @param fd Pointer to the file descriptor.
 * @param offset The offset to seek to.
 * @param whence The reference point for the offset. i.e. FILE_SEEK_SET, FILE_SEEK_CUR, FILE_SEEK_END.
 * @return 0 on success, negative error code on failure.
 */
int fat16_seek(file_descriptor_t* fd, int32_t offset, file_seek_mode_t whence) {
    if (!fd || !fd->fs) {
        return -EBADF; // Bad file descriptor
    }
    fat_file_directory_representation_t* file_rep = (fat_file_directory_representation_t*)fd->fs_private_data;
    if (!file_rep || file_rep->type != FAT_DIRECTORY_ENTRY_TYPE_FILE) {
        return -EBADF; // Bad file descriptor
    }
    uint32_t new_pos = 0;
    switch (whence) {
        case FILE_SEEK_SET:
            new_pos = (uint32_t)offset;
            break;
        case FILE_SEEK_CUR:
            new_pos = file_rep->current_pos + (uint32_t)offset;
            break;
        case FILE_SEEK_END:
            // Not supported yet, as we don't have file size tracking implemented
        default:
            return -EINVAL; // Invalid argument
    }
    // Check for negative position
    if ((int32_t)new_pos < 0) {
        return -EINVAL; // Invalid argument
    }
    file_rep->current_pos = new_pos;
    return 0; // Success
}

int fat16_stat(file_descriptor_t* fd, file_state_t* out_state) {
    if (!fd || !fd->fs) {
        return -EBADF; // Bad file descriptor
    }
    fat_file_directory_representation_t* file_rep = (fat_file_directory_representation_t*)fd->fs_private_data;
    if (!file_rep || file_rep->type != FAT_DIRECTORY_ENTRY_TYPE_FILE) {
        return -EBADF; // Bad file descriptor
    }
    fat_directory_entry_t* entry = file_rep->sfn_entry;
    // In this implementation, files are read-only.
    // For FAT write support, additional logic would be needed.
    // However, from the learning-oriented perspective, reading from FAT16 is quite sufficient.
    out_state->flags = FILE_STATE_READ_ONLY; // FAT16 files are read-only in this implementation
    out_state->file_size = entry->file_size;
    return 0; // Success
}

int fat16_close(file_descriptor_t* fd) {
    if (!fd || !fd->fs) {
        return -EBADF; // Bad file descriptor
    }
    fat_file_directory_representation_t* file_rep = (fat_file_directory_representation_t*)fd->fs_private_data;
    if (file_rep) {
        fat16_free_file_directory_representation(file_rep);
    }
    return 0; // Success
}

/**
 * @brief Initialize the FAT16 file system.
 * @return Pointer to the initialized FAT16 file system structure.
 */
file_system_t* fat16_init() {
    return &fat16_fs;
}