/**
 * @file fat_common.h
 * @brief Common definitions for FAT file systems.
 */

#ifndef __FAT_COMMON_H__
#define __FAT_COMMON_H__

#include <stdint.h>
#include "disk/streamer.h"

typedef enum {
    FAT_DIRECTORY_ENTRY_TYPE_DIRECTORY = 0,
    FAT_DIRECTORY_ENTRY_TYPE_FILE
} fat_directory_entry_type_t;

// FAT file attributes
#define FAT_FILE_ATTR_READ_ONLY 0x01
#define FAT_FILE_ATTR_HIDDEN 0x02
#define FAT_FILE_ATTR_SYSTEM 0x04
#define FAT_FILE_ATTR_VOLUME_LABEL 0x08
#define FAT_FILE_ATTR_LONG_NAME 0x0F
#define FAT_FILE_ATTR_DIRECTORY 0x10
#define FAT_FILE_ATTR_ARCHIVE 0x20
#define FAT_FILE_ATTR_DEVICE 0x40
#define FAT_FILE_ATTR_RESERVED 0x80

/* Common header structure for FAT file systems */
// The following definitions specify the binary layout of each region in a FAT volume.
// This structure is used to read the BPB(BIOS Parameter Block)/headers of FAT12, FAT16, and FAT32
typedef struct fat_common_header {
    uint8_t jump_boot[3];           // Jump instruction to boot code
    char oem_name[8];               // OEM Name
    uint16_t bytes_per_sector;      // Bytes per sector
    uint8_t sectors_per_cluster;    // Sectors per cluster
    uint16_t reserved_sector_count; // Number of reserved sectors
    uint8_t num_fats;               // Number of FATs
    uint16_t root_entry_count;      // Number of root directory entries (FAT12/16)
    uint16_t total_sectors_16;      // Total sectors (if zero, use total_sectors_32)
    uint8_t media;                  // Media descriptor
    uint16_t fat_size_16;           // Sectors per FAT (FAT12/16)
    uint16_t sectors_per_track;     // Sectors per track (for BIOS)
    uint16_t num_heads;             // Number of heads (for BIOS)
    uint32_t hidden_sectors;        // Hidden sectors
    uint32_t total_sectors_32;      // Total sectors (if total_sectors_16 is zero)
} __attribute__((packed)) fat_common_header_t;

// The extended header for FAT16 follows the common header
typedef struct fat16_extended_header {
    uint8_t drive_number;          // Drive number
    uint8_t reserved1;             // Reserved
    uint8_t boot_signature;        // Boot signature (0x29)
    uint32_t volume_id;            // Volume ID
    char volume_label[11];         // Volume label
    char file_system_type[8];      // File system type (e.g., "FAT16   ")
} __attribute__((packed)) fat16_extended_header_t;

// The extended header for FAT32 follows the common header
typedef struct fat32_extended_header {
    uint32_t fat_size_32;          // Sectors per FAT (FAT32)
    uint16_t ext_flags;            // Extended flags
    uint16_t fs_version;           // File system version
    uint32_t root_cluster;         // First cluster of root directory
    uint16_t fs_info;              // Sector number of FSInfo structure
    uint16_t backup_boot_sector;   // Sector number of backup boot sector
    uint8_t reserved[12];          // Reserved
    uint8_t drive_number;          // Drive number
    uint8_t reserved1;             // Reserved
    uint8_t boot_signature;        // Boot signature (0x29)
    uint32_t volume_id;            // Volume ID
    char volume_label[11];         // Volume label
    char file_system_type[8];      // File system type (e.g., "FAT32   ")
} __attribute__((packed)) fat32_extended_header_t;

// FAT directory entry structure for the Short File Name (SFN) format.
typedef struct fat_directory_entry {
    char name[8];                  // File name
    char ext[3];                   // File extension
    uint8_t attributes;            // File attributes (refer to FAT_FILE_ATTR_* defined in this file)
    uint8_t reserved;              // Reserved for Windows NT and DOS
    uint8_t creation_time_tenths;  // Creation time, values from 0 to 199 (in 10ms increments). Provide sub-second resolution.
    uint16_t creation_time;        // Creation time, hours(bits 11-15), minutes(bits 5-10), seconds(bits 0-4, in 2-second increments)
    uint16_t creation_date;        // Creation date, year(bits 9-15, offset from 1980), month(bits 5-8), day(bits 0-4)
    uint16_t last_access_date;     // Last access date, same format as creation_date
    uint16_t first_cluster_high;   // High word of first cluster (FAT32, 0 for FAT16) which contains the file's access rights and other flags
    uint16_t last_mod_time;        // Last modification time, same format as creation_time
    uint16_t last_mod_date;        // Last modification date, same format as creation_date
    uint16_t first_cluster_low;    // Low word of first cluster, which contains the starting cluster number of the file. 0 indicates an empty file.
    uint32_t file_size;            // File size in bytes, 0 for directories
} __attribute__((packed)) fat_directory_entry_t;

// FAT directory entry for Long File Name (LFN) format.
// Note: LFN entries always precede the corresponding SFN entry.
typedef struct fat_lfn_entry {
    uint8_t order;                 // Order of this entry in the sequence
    uint16_t name1[5];            // First 5 characters of the long file name (UTF-16)
    uint8_t attributes;            // Attributes (always 0x0F for LFN)
    uint8_t type;                  // Type (always 0 for LFN)
    uint8_t checksum;              // Checksum of the short file name
    uint16_t name2[6];            // Next 6 characters of the long file name (UTF-16)
    uint16_t first_cluster_low;    // Must be zero for LFN entries
    uint16_t name3[2];            // Last 2 characters of the long file name (UTF-16)
} __attribute__((packed)) fat_lfn_entry_t;

/**
 * The following structure is a composite used for easier handling of
 * FAT headers, directories, and related structures.
 */
// FAT header structure combining common and extended headers
typedef struct fat_header {
    fat_common_header_t common;    // Common FAT header
    union {
        fat16_extended_header_t fat16; // FAT16 extended header
        fat32_extended_header_t fat32; // FAT32 extended header
    } extended;
} fat_header_t;

typedef struct fat_directory {
    fat_directory_entry_t* entries; // Pointer to an array of directory entries
    uint32_t in_use_entry_count;    // Number of in-use entries in the directory
    uint32_t start_pos;             // Starting position (sector) of the directory
    uint32_t end_pos;               // Ending position (sector) of the directory
} fat_directory_t;

// FAT file or directory representation
typedef struct fat_file_directory_representation {
    union {
        fat_directory_entry_t* sfn_entry;        // Pointer to Short File Name entry
        fat_lfn_entry_t* lfn_entry;              // Pointer to Long File Name entry
        fat_directory_t* directory;              // Pointer to a directory structure
    };
    fat_directory_entry_type_t type;             // Type of the directory entry
    uint32_t current_pos;                        // Current position (in bytes) within the file or directory
                                                 // which might be in a cluster chain
                                                 // and used for read/write operations
} fat_file_directory_representation_t;

typedef struct fat_fs_private_data {
    fat_header_t header;                  // FAT header information
    fat_directory_t root_directory;       // Root directory information
    disk_streamer_t* cluster_streamer;    // Streamer for reading clusters
    disk_streamer_t* fat_read_streamer;   // Streamer for reading FAT tables
    disk_streamer_t* directory_streamer;  // Streamer for reading directories
} fat_fs_private_data_t;

#endif // __FAT_COMMON_H__