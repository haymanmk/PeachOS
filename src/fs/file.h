#ifndef __FILE_H__
#define __FILE_H__

#include <stdint.h>
#include <stddef.h>
#include "config.h"
#include "status.h"
#include "disk/disk.h"
#include "fs/pparser.h"

// Forward declaration to avoid circular dependency
// between file.h and disk.h
typedef struct disk disk_t;

/* Type definitions */
typedef enum {
    SEEK_SET = 0,
    SEEK_CUR = 1,
    SEEK_END = 2
} file_seek_mode_t;

typedef enum {
    FILE_MODE_INVALID = 0,
    FILE_MODE_READ = 1 << 0,
    FILE_MODE_WRITE = 1 << 1,
    FILE_MODE_APPEND = 1 << 2
} file_mode_t;

// Function pointers
typedef void*(*file_open_func_t)(disk_t* disk, path_part_t* path_part, file_mode_t mode);
typedef int(*file_resolve_func_t)(disk_t* disk);

// File system structure
typedef struct file_system {
    char name[16];               // Name of the file system
    file_resolve_func_t resolve; // Function to resolve the file system.
                                 // Returns 0 if loaded disk matches this FS.
    file_open_func_t open;       // Function to open a file in this file system.
} file_system_t;

// file descriptor structure
typedef struct file_descriptor {
    uint32_t id;               // Index in the file descriptor table
    file_system_t* fs;         // Pointer to the file system handling this file
    disk_t* disk;              // Pointer to the disk where the file resides

    // Private data for the file system (used internally by the FS)
    void* fs_private_data;
} file_descriptor_t;

/* Exported functions */
error_t file_init();
error_t file_insert_file_system(file_system_t* fs);
file_system_t* file_system_resolve(disk_t* disk);

file_descriptor_t* file_open(const char* path, file_mode_t mode);
int file_close(file_descriptor_t* fd);
size_t file_read(file_descriptor_t* fd, size_t size, void* buffer);
size_t file_write(file_descriptor_t* fd, size_t size, const void* buffer);

#endif // __FILE_H__