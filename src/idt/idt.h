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

/**
 * @brief Structure representing the CPU state pushed onto the stack during an interrupt.
 * @note  For 32-bit x86 architecture, the stack frame has 4-byte alignment.
 *        The order of the registers is based on the sequence they are pushed in the assembly stubs.
 *        For the details, refer to the interrupt handling assembly code.
 */
typedef struct idt_interrupt_stack_frame {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp; // Original ESP before pusha
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t user_esp; // Only pushed when transitioning from user to kernel mode
    uint32_t ss;       // Only pushed when transitioning from user to kernel mode
} __attribute__((packed)) idt_interrupt_stack_frame_t;

/**
 * @brief Type definition for an interrupt handler function.
 * @param frame Pointer to the interrupt stack frame.
 * @return A void pointer (can be used to return values if needed).
 */
typedef void* (*idt_interrupt_handler_t)(idt_interrupt_stack_frame_t* frame);

extern void idt_enable_interrupts();
extern void idt_disable_interrupts();

void idt_init();
int idt_register_interrupt_handler(uint16_t interrupt_number, idt_interrupt_handler_t handler);
void idt_general_interrupt_handler_c(uint16_t interrupt_number, idt_interrupt_stack_frame_t* frame);

#endif // __IDT_H__
