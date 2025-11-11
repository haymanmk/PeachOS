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
    for (uint8_t i = 0; i < DISK_MAX_DISKS; i++) {
        // Detect and initialize disks, populate disk_list
        // Allocate and set up disk_t structures
        disk_t* new_disk = (disk_t*)kheap_zmalloc(sizeof(disk_t));
        if (!new_disk) {
            return -ENOMEM; // Memory allocation error
        }
        new_disk->uid = DISK_MAX_DISKS - i - 1; // Assign unique ID
        new_disk->type = DISK_TYPE_ATA; // Example type
        new_disk->limit = 10; // number of sectors (sector size is equal to DISK_SECTOR_SIZE, e.g., 512 bytes)
        new_disk->partitions = NULL;
        new_disk->next = disk_list;
        disk_list = new_disk; // Add to the front of the list, link disks forward
    }

    return 0; // Return 0 on success
}

/**
 * @brief Retrieve a disk by its unique ID.
 * @param uid The unique ID of the disk.
 * @return Pointer to the disk_t structure if found, NULL otherwise.
 */
disk_t *disk_get_by_uid(uint8_t uid) {
    // Code to retrieve a disk by its unique ID goes here
    // Iterate through the linked list of disks for the matching UID
    disk_t* current = disk_list;
    while (current) {
        if (current->uid == uid) {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

int disk_read_lba(disk_t* disk, uint32_t lba, uint32_t count, void* buffer) {
    // Dispatch to the appropriate disk read function based on disk type
    // Check count does not exceed disk limit
    if (count > disk->limit) {
        return -EINVAL; // Invalid argument
    }

    // Call the appropriate read function based on disk type
    switch (disk->type) {
        case DISK_TYPE_ATA:
            return disk_read_lba_ata(lba, count, buffer);
        // Add cases for other disk types as needed
        default:
            return -EINVAL; // Unsupported disk type
    }
}

int disk_add_partition(disk_t* disk, const char* name, uint8_t from, uint8_t size) {
    // Examine if the name is unique within the disk's partitions
    disk_partition_t* current = disk->partitions;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return -EINVAL; // Partition name already exists
        }
        current = current->next;
    }
    // Create a new partition
    disk_partition_t* new_partition = (disk_partition_t*)kheap_zmalloc(sizeof(disk_partition_t));
    if (!new_partition) {
        return -ENOMEM; // Memory allocation error
    }
    strncpy(new_partition->name, name, DEV_NAME_SIZE);
    new_partition->from = from;
    new_partition->size = size;
    new_partition->next = disk->partitions;
    disk->partitions = new_partition; // Add to the front of the partition list

    return 0; // Success
}