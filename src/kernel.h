#ifndef __KERNEL_H__
#define __KERNEL_H__

void panic(const char* message);
void kernel_main();
void kernel_page();

/**
 * @brief Restore segment registers (DS, ES, FS, GS) to the kernel data segment.
 * @note  This function is typically implemented in assembly.
 */
void kernel_restore_segment_registers_to_kernel_data();

#endif