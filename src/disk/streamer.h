#ifndef __STREAMER_H__
#define __STREAMER_H__

#include <stdint.h>
#include "disk/disk.h"

/* Type definitions */
typedef struct disk_streamer {
    uint32_t pos;      // Current position in bytes
    disk_t *disk;        // Associated disk
} disk_streamer_t;

/* Exported functions */

disk_streamer_t* disk_streamer_create(uint8_t disk_uid);
int disk_streamer_seek(disk_streamer_t* streamer, uint32_t pos);
int disk_streamer_read(disk_streamer_t* streamer, uint32_t size, void *buffer);
void disk_streamer_destroy(disk_streamer_t* streamer);

#endif // __STREAMER_H__