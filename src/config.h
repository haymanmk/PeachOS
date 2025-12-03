#ifndef __CONFIG_H__
#define __CONFIG_H__

// Configuration options for the kernel and system
/* Interrupts */
/**
 * Note: __PIC1_VECTOR_OFFSET is used to remap the PIC interrupt vectors
 *       to avoid conflicts with Intel CPU exceptions (0-31).
 */
#define IDT_SIZE 256
#define __PIC1_COMMAND_PORT 0x20
#define __PIC1_DATA_PORT 0x21
#define __PIC1_VECTOR_OFFSET 0x20

// System Call
// ISR 0x80
#define ISR80H_MAX_COMMANDS 1024 // Maximum number of system call commands for ISR 0x80

/* GDT */
#define GDT_MAX_ENTRIES 6
#define RPL_KERNEL 0x0
#define RPL_USER 0x3
#define KERNEL_CODE_SELECTOR 0x08
#define KERNEL_DATA_SELECTOR 0x10
#define USER_CODE_SELECTOR 0x18
#define USER_DATA_SELECTOR 0x20
/* GDT - TSS */
#define TSS_SELECTOR 0x28

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

// Stack for programs
#define PROGRAM_VIRTUAL_ADDRESS 0x400000 // 4 MB
#define PROGRAM_VIRTUAL_STACK_SIZE_BYTES (16 * 1024) // 16 KB
#define PROGRAM_VIRTUAL_STACK_TOP_ADDRESS 0x3FF000 // Just below 4 MB
#define PROGRAM_VIRTUAL_STACK_BOTTOM_ADDRESS (PROGRAM_VIRTUAL_STACK_TOP_ADDRESS - PROGRAM_VIRTUAL_STACK_SIZE_BYTES)
#define PROGRAM_MAX_ALLOCATIONS 1024 // Maximum number of memory allocations per program which can be tracked
#define PROGRAM_MAX_PROCESSES 12 // Maximum number of processes in the system

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
#define FS_MAX_FILE_NAME_LENGTH 64

#endif // __CONFIG_H__
