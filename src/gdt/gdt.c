#include "gdt.h"
#include "kernel.h"

void gdt_encode_entry(gdt_entry_t* entry, gdt_structured_t* structured_entry) {
    entry->limit_low    = (structured_entry->limit & 0xFFFF);
    entry->base_low     = (structured_entry->base & 0xFFFF);
    entry->base_middle  = (structured_entry->base >> 16) & 0xFF;
    entry->access       = structured_entry->type;
    entry->granularity  = ((structured_entry->limit >> 16) & 0x0F);
    entry->granularity |= (0xC0); // Set granularity to 4KB and 32-bit mode
    entry->base_high    = (structured_entry->base >> 24) & 0xFF;
}

void gdt_init(gdt_entry_t* out_gdt_entries, gdt_structured_t* structured_gdt, uint16_t total_entries) {
    for (uint16_t i = 0; i < total_entries; i++) {
        gdt_encode_entry(&out_gdt_entries[i], &structured_gdt[i]);
    }
}