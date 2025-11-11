#ifndef __DISK_H__
#define __DISK_H__

#include <stdint.h>
#include <stddef.h>
#include "config.h"
#include "status.h"

#define DEV_NAME_SIZE 32 // the max. number of chars for a device's name

/* Type definitions */
// Partition
typedef struct partition {
    char name[DEV_NAME_SIZE]; // unique name of this partition
    uint8_t from; // first sector number of this partition (zero-based index)
    uint8_t size; // the number of sectors in this partition
    struct partition* next;
} disk_partition_t;

// Disk
typedef enum {
    DISK_TYPE_UNKNOWN = 0,
    DISK_TYPE_ATA,
    DISK_TYPE_SATA,
    DISK_TYPE_NVME,
    DISK_TYPE_USB,
    // Add more disk types as needed
} disk_type_t;

typedef struct disk {
    uint8_t uid; // the unique ID of this disk
    disk_type_t type; // the type of this disk
    disk_partition_t* partitions; // linked list of partitions on this disk
    uint32_t limit; // the total number of sectors on this disk
    struct disk* next;
} disk_t;

/* Exported functions */
int disk_init();
disk_t* disk_get_by_uid(uint8_t uid);
int disk_read_lba(disk_t* disk, uint32_t lba, uint32_t count, void* buffer);
int disk_add_partition(disk_t* disk, const char* name, uint8_t from, uint8_t size);

#endif // __DISK_H__