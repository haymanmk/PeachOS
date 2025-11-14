#include "disk.h"
#include "io/io.h"
#include "memory/heap/kheap.h"
#include "utils/string.h"

/**
 * @file disk.c
 * @brief Disk management implementation.
 */

static disk_t* disk_list = NULL; // Head of the linked list of disks

 /**
  * @brief Read sectors from the disk using LBA addressing through ATA.
  * @param lba The starting Logical Block Addressing (LBA) sector number.
  * @param count The number of sectors to read.
  * @param buffer The buffer to store the read data.
  * @return 0 on success, error code otherwise.
  */
int disk_read_lba_ata(uint32_t lba, uint32_t count, void* buffer) {
    // Code to read sectors using LBA goes here
    outsb(0x1F6, (lba >> 24) | 0xE0); // Send LBA high bits and set bit 6 for LBA mode
    outsb(0x1F2, count); // Send the sector count
    outsb(0x1F3, (uint8_t)(lba & 0xFF)); // Send the low byte of LBA
    outsb(0x1F4, (uint8_t)(lba >> 8)); // Send the middle byte of LBA
    outsb(0x1F5, (uint8_t)(lba >> 16)); // Send the high byte of LBA
    outsb(0x1F7, 0x20); // Send the command to read sectors with retry

    // Wait for the disk to be ready and read the data into the buffer
    // This is a simplified example; proper error handling and waiting should be implemented
    for (uint32_t i = 0; i < count; i++) {
        // Wait for the disk to signal that data is ready
        while ((insb(0x1F7) & 0x08) == 0);
        // Read a sector (512 bytes)
        for (uint32_t j = 0; j < DISK_SECTOR_SIZE / 2; j++) {
            ((uint16_t*)buffer)[i * (DISK_SECTOR_SIZE / 2) + j] = insw(0x1F0);
        }
    }

    return 0; // Return 0 on success
}

/**
 * @brief Initialize the disk subsystem.
 * @return 0 on success, error code otherwise.
 */
int disk_init() {
    // Disk initialization code goes here
    disk_t* new_disk = (disk_t*)kheap_zmalloc(sizeof(disk_t));
    if (!new_disk) {
        return -ENOMEM; // Memory allocation error
    }
    new_disk->uid = 0; // Assign a unique ID
    new_disk->type = DISK_TYPE_ATA; // Set disk type
    new_disk->sector_size = DISK_SECTOR_SIZE; // Set sector size
    new_disk->fs = file_system_resolve(new_disk); // Resolve file system

    disk_list = new_disk; // Add to the disk list

    return 0; // Return 0 on success
}

/**
 * @brief Retrieve a disk by its unique ID.
 * @param uid The unique ID of the disk.
 * @return Pointer to the disk_t structure if found, NULL otherwise.
 */
disk_t *disk_get_by_uid(uint8_t uid) {
    // Code to retrieve a disk by its unique ID goes here
    if (!disk_list) {
        return NULL; // No disks available
    }
    if (disk_list->uid == uid) {
        return disk_list; // Found the disk
    }

    return NULL;
}

/**
 * @brief Read sectors from the specified disk using LBA addressing.
 * @param disk Pointer to the disk to read from.
 * @param lba The starting Logical Block Addressing (LBA) sector number.
 * @param count The number of sectors to read.
 * @param buffer The buffer to store the read data.
 * @return 0 on success, error code otherwise.
 */
int disk_read_lba(disk_t* disk, uint32_t lba, uint32_t count, void* buffer) {
    // Dispatch to the appropriate disk read function based on disk type

    // Call the appropriate read function based on disk type
    switch (disk->type) {
        case DISK_TYPE_ATA:
            return disk_read_lba_ata(lba, count, buffer);
        // Add cases for other disk types as needed
        default:
            return -EINVAL; // Unsupported disk type
    }
}
