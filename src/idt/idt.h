#ifndef __IDT_H__
#define __IDT_H__
#include <stdint.h>

#define IDT_GATE_TYPE_TASK_GATE      0x5   // 0b0101
#define IDT_GATE_TYPE_INT_GATE_16    0x6   // 0b0110
#define IDT_GATE_TYPE_TRAP_GATE_16   0x7   // 0b0111
#define IDT_GATE_TYPE_INT_GATE_32    0xE   // 0b1110
#define IDT_GATE_TYPE_TRAP_GATE_32   0xF   // 0b1111

#define IDT_DPL_RING0               0x00  // 0b00 << 5
#define IDT_DPL_RING1               0x20  // 0b01 << 5
#define IDT_DPL_RING2               0x40  // 0b10 << 5
#define IDT_DPL_RING3               0x60  // 0b11 << 5

#define IDT_PRESENT                 0x80  // 1 << 7

// Types and function prototypes for Interrupt Descriptor Table (IDT) management
/**
 * @brief Structure representing an entry in the Interrupt Descriptor Table (IDT).
 */
typedef struct IDTEntry {
    uint16_t offset_low;    // Lower 16 bits of handler function address (Interrupt Service Routine)
    uint16_t selector;      // Kernel segment selector
    uint8_t  zero;          // This must always be zero (reserved)
    uint8_t  type_attr;     // Type and attributes
    uint16_t offset_high;   // Upper 16 bits of handler function address
} __attribute__((packed)) idt_entry_t;

/**
 * @brief Pointer structure to the IDT, used by the `lidt` instruction.
 */
typedef struct IDTPointer {
    uint16_t limit;         // Size of the IDT in bytes - 1
    uint32_t base;          // Base address of the first element in the IDT
} __attribute__((packed)) idt_ptr_t;

extern void idt_enable_interrupts();
extern void idt_disable_interrupts();

void idt_init();

#endif // __IDT_H__
