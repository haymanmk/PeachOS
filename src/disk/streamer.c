#include "streamer.h"
#include "config.h"
#include "status.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "disk/disk.h"

/**
 * @file src/disk/streamer.c
 * @brief Implementation of disk streaming functionalities.
 *        The features of streaming let the disk read/write data
 *        in a smaller, continuous manner instead of large chunks like sectors.
 *        Although the underlying mechanism still relies on sector-based operations,
 *        streaming provides a more flexible and efficient way to handle data transfers.
 */

 /**
  * @brief Create a disk streamer for the specified disk UID.
  * @param disk_uid The unique identifier of the disk.
  * @return Pointer to the created disk_streamer_t, or NULL on failure.
  */
disk_streamer_t* disk_streamer_create(uint8_t disk_uid) {
    disk_t* disk = disk_get_by_uid(disk_uid);
    if (!disk) {
        return NULL; // Disk not found
    }

    disk_streamer_t* streamer = (disk_streamer_t*)kheap_zmalloc(sizeof(disk_streamer_t));
    if (!streamer) {
        return NULL; // Memory allocation failed
    }

    streamer->pos = 0;
    streamer->disk = disk;
    return streamer;
}

/**
 * @brief Seek to a specific LBA position in the disk streamer.
 *        NOTE: This function should be called before any read/write operations.
 *              It updates the current LBA position of the streamer,
 *              which means subsequent read/write operations will start from this position.
 * @param streamer Pointer to the disk streamer.
 * @param pos The LBA position to seek to (in bytes).
 * @return 0 on success, non-zero on failure.
 */
int disk_streamer_seek(disk_streamer_t* streamer, uint32_t pos) {
    if (!streamer) {
        return -EINVAL; // Invalid argument
    }

    streamer->pos = pos;
    return 0;
}

/**
 * @brief Read data from the disk streamer into the provided buffer.
 * @param streamer Pointer to the disk streamer.
 * @param size Number of bytes to read.
 * @param buffer Pointer to the buffer where data will be stored.
 * @return 0 on success, non-zero on failure.
 */
int disk_streamer_read(disk_streamer_t* streamer, uint32_t size, void *buffer) {
    if (!streamer || !buffer || size == 0) {
        return -EINVAL; // Invalid argument
    }

    uint32_t sector_size = streamer->disk->sector_size;
    uint32_t start_lba = streamer->pos / sector_size; // in sectors
    uint32_t offset = streamer->pos % sector_size; // in bytes
    uint32_t total_bytes_read = 0;
    uint8_t buffer_sector[sector_size];

    // Read loop. Here we read sector by sector and copy the required bytes to the user buffer.
    // We did not implement the recursive manner to keep stack usage low.
    while (total_bytes_read < size) {
        // Read one sector into the temporary buffer
        if (disk_read_lba(streamer->disk, start_lba, 1, buffer_sector) != 0) {
            return -EIO; // Disk read error
        }
        // Calculate how many bytes to copy from this sector
        uint32_t bytes_to_copy = sector_size - offset;
        if (bytes_to_copy > (size - total_bytes_read)) {
            // Adjust if remaining bytes are less than bytes_to_copy
            bytes_to_copy = size - total_bytes_read;
        }

        // Copy the data from the buffer to the user buffer
        memcpy(buffer + total_bytes_read, buffer_sector + offset, bytes_to_copy);
        total_bytes_read += bytes_to_copy;
        streamer->pos += bytes_to_copy;

        // If we've read the entire requested size, we can stop
        if (total_bytes_read == size) {
            break;
        }

        // Move to the next sector
        start_lba++;
        offset = 0; // Reset offset for the next sector
    }

    return 0;
}

/**
 * @brief Destroy the disk streamer and free associated resources.
 * @param streamer Pointer to the disk streamer to be destroyed.
 */
void disk_streamer_destroy(disk_streamer_t* streamer) {
    if (!streamer) {
        return;
    }

    kheap_free(streamer);
}
