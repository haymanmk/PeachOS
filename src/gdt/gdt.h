#ifndef __GDT_H
#define __GDT_H

#include <stdint.h>

typedef struct {
    uint16_t limit_low;      // Lower 16 bits of limit
    uint16_t base_low;       // Lower 16 bits of base
    uint8_t  base_middle;    // Next 8 bits of base
    uint8_t  access;         // Access flags
    uint8_t  granularity;    // Granularity and upper 4 bits of limit
    uint8_t  base_high;      // Upper 8 bits of base
} __attribute__((packed)) gdt_entry_t;

/**
 * A shorthand structure representing for GDT entry
 */
typedef struct {
    uint32_t base;    // Address of the first gdt_entry_t
    uint32_t limit;   // limit of the segment
    uint8_t  type;    // Type of access. e.g., kernel/user code/data segment
} gdt_structured_t;

/**
 * Load the GDT with the provided entries and size.
 * Note: This function is written in assembly.
 *
 * @param gdt_entries Pointer to the array of GDT entries.
 * @param limit Size of the GDT - 1.
 */
void gdt_load(gdt_entry_t* gdt_entries, uint16_t limit);
void gdt_init(gdt_entry_t* out_gdt_entries, gdt_structured_t* structured_gdt, uint16_t total_entries);

#endif // __GDT_H