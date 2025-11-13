#ifndef __CONFIG_H__
#define __CONFIG_H__

// Configuration options for the kernel and system
/* Interrupts */
#define IDT_SIZE 256
#define __PIC1_COMMAND_PORT 0x20
#define __PIC1_DATA_PORT 0x21
#define __PIC1_VECTOR_OFFSET 0x20

#define KERNEL_CODE_SELECTOR 0x08
#define KERNEL_DATA_SELECTOR 0x10

/* Memory */
// Paging
#define PAGE_DIRECTORY_SIZE 4096
#define PAGE_TABLE_SIZE 4096
#define PAGE_SIZE 4096
#define PAGE_ENTRIES_PER_TABLE 1024

// Kernel Heap
// Allocate 100 MB for the kernel heap
#define KERNEL_HEAP_BLOCK_SIZE PAGE_SIZE // This should match the page size
#define KERNEL_HEAP_SIZE_BYTES (100 * 1024 * 1024)
// NOTE: KERNEL_HEAP_MAX_BLOCKS must not exceed 0xFFFFFFFF-1 due to block index type limitation
#define KERNEL_HEAP_MAX_BLOCKS (KERNEL_HEAP_SIZE_BYTES / KERNEL_HEAP_BLOCK_SIZE)
// Note: The addresses for the kernel heap are chosen according to the memory map table
//       collected in OSDev Wiki: https://wiki.osdev.org/Memory_Map_(x86),
//       which can differ between systems.
#define KERNEL_HEAP_ADDRESS 0x01000000
#define KERNEL_HEAP_TABLE_ADDRESS 0x00007E00

/* Disk */
#define DISK_SECTOR_SIZE 512
#define DISK_MAX_DISKS 1
#define DISK_MAX_PARTITIONS 4

/* File System */
// Path Parser
#define PATH_MAX_PARTS 32 // Maximum number of parts in a path, e.g., /part1/part2/part3 ...
#define PATH_MAX_PART_NAME_LENGTH 64 // Maximum length of a part name

// File System
#define FS_MAX_FILE_SYSTEMS 8
#define FS_MAX_FILE_DESCRIPTORS 256

#endif // __CONFIG_H__
