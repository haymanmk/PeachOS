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

#endif // __CONFIG_H__
