#include "idt.h"
#include "config.h"
#include "memory/memory.h"
#include "utils/stdio.h"
#include "io/io.h"

// Define gate type for 32-bit interrupt gate with Ring 3 privilege and present bit set
#define GATE_TYPE_INT_32 (IDT_GATE_TYPE_INT_GATE_32 | IDT_DPL_RING3 | IDT_PRESENT)

/**
 * Only for demostration purposes to detect the keyboard interrupt (IRQ1).
 */
extern void int21h_handler_asm();
void int21h_handler_c() {
    printf("Keyboard Interrupt Received (IRQ1)!\n");

    // Send End of Interrupt (EOI) signal to PICs
    outb(__PIC1_COMMAND_PORT, 0x20); // EOI to Master PIC
}

idt_entry_t idt[IDT_SIZE];
idt_ptr_t idt_ptr;

extern void idt_load(uint32_t idt_ptr_address);
extern void idt_interrupt_stub();

void idt_div_by_zero_handler() {
    printf("Division by Zero Exception!\n");

    // Halt the system or take appropriate action
    while (1);
}

/**
 * @brief Sets an entry(gate descriptor) in the Interrupt Descriptor Table (IDT).
 * @param interrupt_number The interrupt number to set.
 * @param handler_address The address of the interrupt handler function.
 * @param selector The kernel segment selector which must point to the code segment in GDT.
 * @param type_attr The type and attributes for the IDT entry.
 *        Gate types: (bits 0-3 of type_attr)
 *          - 0b0101 or 0x5: Task Gate, note that the offset fields are ignored.
 *          - 0x0110 or 0x6: 16-bit Interrupt Gate.
 *          - 0x0111 or 0x7: 16-bit Trap Gate.
 *          - 0x1110 or 0xE: 32-bit Interrupt Gate.
 *          - 0x1111 or 0xF: 32-bit Trap Gate.
 *        Descriptor Privilege Level (DPL): (bits 5-6 of type_attr)
 *          - 0b00: Ring 0 (highest privilege)
 *          - 0b01: Ring 1
 *          - 0b10: Ring 2
 *          - 0b11: Ring 3 (lowest privilege)
 *        Present Bit: (bit 7 of type_attr)
 *          - 0: Not present
 *          - 1: Present
 */
void idt_set_gate(uint8_t interrupt_number, const uint32_t handler_address, uint16_t selector, uint8_t type_attr) {
    idt[interrupt_number].offset_low = (uint16_t)(handler_address & 0xFFFF);
    idt[interrupt_number].selector = selector;
    idt[interrupt_number].zero = 0;
    idt[interrupt_number].type_attr = type_attr;
    idt[interrupt_number].offset_high = (uint16_t)((handler_address >> 16) & 0xFFFF);
}

void idt_init() {
    // Initialize the Interrupt Descriptor Table (IDT)
    // This is a placeholder implementation
    // Actual IDT setup code would go here
    memset(&idt, 0, sizeof(idt_entry_t) * IDT_SIZE);
    idt_ptr.limit = sizeof(idt_entry_t) * IDT_SIZE - 1;
    idt_ptr.base = (uint32_t)&idt;

    // Set up IDT entries with null handlers as stubs
    for (uint16_t i = 0; i < IDT_SIZE; i++) {
        idt_set_gate(i, (uint32_t)idt_interrupt_stub, KERNEL_CODE_SELECTOR, GATE_TYPE_INT_32);
    }

    // Assign interrupt handlers here
    idt_set_gate(0, (uint32_t)idt_div_by_zero_handler, KERNEL_CODE_SELECTOR, GATE_TYPE_INT_32);

    // Example: Set keyboard interrupt handler (IRQ1)
    idt_set_gate(0x21, (uint32_t)int21h_handler_asm, KERNEL_CODE_SELECTOR, GATE_TYPE_INT_32);

    // Load the IDT using the lidt instruction
    idt_load((uint32_t)&idt_ptr);
}