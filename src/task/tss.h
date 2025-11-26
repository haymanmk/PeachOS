#ifndef __TSS_H__
#define __TSS_H__

#include <stdint.h>
// Task State Segment (TSS) structure
typedef struct {
    uint32_t prev_tss;   // Previous TSS (if hardware task switching is used)
    uint32_t esp0;       // Stack pointer to load when changing to kernel mode
    uint32_t ss0;        // Stack segment to load when changing to kernel mode
    uint32_t esp1;       // Unused
    uint32_t ss1;        // Unused
    uint32_t esp2;       // Unused
    uint32_t ss2;        // Unused
    uint32_t cr3;       // Page directory base register
    uint32_t eip;       // Instruction pointer
    uint32_t eflags;    // Flags register
    uint32_t eax;       // General purpose registers
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;        // Segment selectors
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;       // Local Descriptor Table segment selector
    uint16_t trap;      // Trap on task switch
    uint16_t iomap_base; // I/O map base address
} __attribute__((packed)) tss_t;

/**
 * @brief Load the TSS segment selector into the task register (TR).
 *        Note: This function is written in assembly.
 * @param tss_segment_selector The segment selector for the TSS in the GDT.
 */
void tss_load(uint16_t tss_segment_selector);

#endif // __TSS_H__