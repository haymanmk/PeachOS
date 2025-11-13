#ifndef __DISK_H__
#define __DISK_H__

#include <stdint.h>
#include <stddef.h>
#include "config.h"
#include "status.h"

#define DEV_NAME_SIZE 32 // the max. number of chars for a device's name

/* Type definitions */

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
    uint32_t sector_size; // size of a sector in bytes
} disk_t;

/* Exported functions */
int disk_init();
disk_t* disk_get_by_uid(uint8_t uid);
int disk_read_lba(disk_t* disk, uint32_t lba, uint32_t count, void* buffer);

#endif // __DISK_H__