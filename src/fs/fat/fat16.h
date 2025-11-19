#ifndef __FAT16_H__
#define __FAT16_H__

#include "fs/file.h"
#include "status.h"

#define FAT16_FAT_ENTRY_SIZE 2 // Each FAT16 entry occupies 2 bytes within the FAT table

/* Macros */

/* Exported functions */
file_system_t* fat16_init();

#endif // __FAT16_H__