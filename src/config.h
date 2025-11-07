#ifndef __CONFIG_H__
#define __CONFIG_H__

// Configuration options for the kernel and system
// Interrupts
#define IDT_SIZE 256
#define __PIC1_COMMAND_PORT 0x20
#define __PIC1_DATA_PORT 0x21
#define __PIC1_VECTOR_OFFSET 0x20

#define KERNEL_CODE_SELECTOR 0x08
#define KERNEL_DATA_SELECTOR 0x10

// Memory
// Allocate 100 MB for the kernel heap
#define KERNEL_HEAP_BLOCK_SIZE 4096
#define KERNEL_HEAP_SIZE_BYTES (100 * 1024 * 1024)
// NOTE: KERNEL_HEAP_MAX_BLOCKS must not exceed 0xFFFFFFFF-1 due to block index type limitation
#define KERNEL_HEAP_MAX_BLOCKS (KERNEL_HEAP_SIZE_BYTES / KERNEL_HEAP_BLOCK_SIZE)
// Note: The addresses for the kernel heap are chosen according to the memory map table
//       collected in OSDev Wiki: https://wiki.osdev.org/Memory_Map_(x86),
//       which can differ between systems.
#define KERNEL_HEAP_ADDRESS 0x01000000
#define KERNEL_HEAP_TABLE_ADDRESS 0x00007E00

#endif // __CONFIG_H__
