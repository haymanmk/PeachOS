#include "idt.h"
#include "config.h"
#include "memory/memory.h"
#include "utils/stdio.h"
#include "io/io.h"
#include "kernel.h"
#include "task/task.h"
#include "status.h"

// Define gate type for 32-bit interrupt gate with Ring 3 privilege and present bit set
#define GATE_TYPE_INT_32 (IDT_GATE_TYPE_INT_GATE_32 | IDT_DPL_RING3 | IDT_PRESENT)

idt_entry_t idt[TOTAL_INTERRUPTS];
idt_ptr_t idt_ptr;
// Table of general interrupt handlers in C
static idt_interrupt_handler_t idt_general_interrupt_handlers[TOTAL_INTERRUPTS];

extern void idt_load(uint32_t idt_ptr_address);
extern void idt_interrupt_stub();
extern void idt_isr80h_handler_asm(); // System call interrupt handler written in assembly
extern void* idt_general_interrupt_handler_table[TOTAL_INTERRUPTS]; // Table of general interrupt handlers implemented in assembly

void idt_div_by_zero_handler() {
    printf("Division by Zero Exception!\n");

    // Halt the system or take appropriate action
    while (1);
}

void idt_page_fault_handler(idt_interrupt_stack_frame_t* frame, uint32_t faulting_address) {
    panic("Page Fault Exception!");
}

void idt_control_protection_fault_handler(idt_interrupt_stack_frame_t* frame) {
    panic("Control Protection Fault Exception!\n");
}

void idt_general_interrupt_handler_c(int interrupt_number, idt_interrupt_stack_frame_t* frame) {
    printf("General Interrupt Received! Interrupt Number: %d\n", interrupt_number);

    // Switch to kernel paging
    kernel_page();
    // If there is a registered handler, call it
    if (idt_general_interrupt_handlers[interrupt_number]) {
        // Save the current task's state such as registers
        task_save_current_state(frame);
        // Call the registered handler
        idt_general_interrupt_handlers[interrupt_number](frame);
    }
    // Return to user paging after interrupt handling
    task_page_current();

    // Send End of Interrupt (EOI) signal to PICs
    if (interrupt_number >= __PIC1_VECTOR_OFFSET && interrupt_number < (__PIC1_VECTOR_OFFSET + 8)) {
        // IRQ from Master PIC
        outb(__PIC1_COMMAND_PORT, 0x20); // EOI to Master PIC
    } else if (interrupt_number >= __PIC2_VECTOR_OFFSET && interrupt_number < (__PIC2_VECTOR_OFFSET + 8)) {
        // IRQ from Slave PIC
        outb(__PIC2_COMMAND_PORT, 0x20); // EOI to Slave PIC
        outb(__PIC1_COMMAND_PORT, 0x20); // EOI to Master PIC
    }
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

/**
 * @brief Initialize the Interrupt Descriptor Table (IDT) with default handlers.
 */
void idt_init() {
    // Initialize the Interrupt Descriptor Table (IDT)
    // This is a placeholder implementation
    // Actual IDT setup code would go here
    memset(&idt, 0, sizeof(idt_entry_t) * TOTAL_INTERRUPTS);
    idt_ptr.limit = sizeof(idt_entry_t) * TOTAL_INTERRUPTS - 1;
    idt_ptr.base = (uint32_t)&idt;

    // Set up IDT entries with null handlers as stubs
    for (uint16_t i = 0; i < TOTAL_INTERRUPTS; i++) {
        idt_set_gate(i, (uint32_t)idt_general_interrupt_handler_table[i], KERNEL_CODE_SELECTOR, GATE_TYPE_INT_32);
    }

    // Assign interrupt handlers here
    idt_set_gate(0, (uint32_t)idt_div_by_zero_handler, KERNEL_CODE_SELECTOR, GATE_TYPE_INT_32);

    // Page Fault Exception (ISR 14)
    idt_set_gate(14, (uint32_t)idt_page_fault_handler, KERNEL_CODE_SELECTOR, GATE_TYPE_INT_32);

    // Control Protection Fault Exception (ISR 21)
    idt_set_gate(21, (uint32_t)idt_control_protection_fault_handler, KERNEL_CODE_SELECTOR, GATE_TYPE_INT_32);

    // Set system call interrupt handler (ISR 0x80)
    idt_set_gate(0x80, (uint32_t)idt_isr80h_handler_asm, KERNEL_CODE_SELECTOR, GATE_TYPE_INT_32);

    // Load the IDT using the lidt instruction
    idt_load((uint32_t)&idt_ptr);
}

int idt_register_interrupt_handler(uint16_t interrupt_number, idt_interrupt_handler_t handler) {
    if (interrupt_number >= TOTAL_INTERRUPTS) {
        return -EINVAL; // Invalid interrupt number
    }
    idt_general_interrupt_handlers[interrupt_number] = handler;
    return ENONE;
}