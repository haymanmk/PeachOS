# Interrupt Descriptor Table (IDT) management
.code32
#include "config.h"

.equ __TOTAL_INTERRUPTS, TOTAL_INTERRUPTS          # Number of interrupts in the system

.section .asm, "ax", @progbits  # Define .asm section as executable code

.extern idt_isr80h_handler_c # External C handler for ISR 0x80
.extern idt_general_interrupt_handler_c # External C handler for general interrupts

.global idt_load
.global idt_enable_interrupts
.global idt_disable_interrupts
.global idt_interrupt_stub
.global idt_isr80h_handler_asm
.global idt_general_interrupt_handler_table

### Macros
.macro idt_interrupt_vector num
    .align 4
    .global idt_interrupt_\num
    .type idt_interrupt_\num, @function
    idt_interrupt_\num:
        pushal                  # Save all 32-bit general-purpose registers
        push %esp          # Push pointer to the interrupt stack frame
        push $\num             # Push the interrupt vector number onto the stack
        call idt_general_interrupt_handler_c # Call the C handler
        addl $8, %esp          # Clean up the stack (remove vector number and frame pointer)
        popal                   # Restore all 32-bit general-purpose registers
        iret                    # Return from interrupt
.endm

# Macro to create an entry in the interrupt handler pointer table
# By following the naming convention of idt_interrupt_<num>,
# we can easily reference the correct handler.
.macro idt_interrupt_array_entry num
    .long idt_interrupt_\num
.endm

### Macros END

# Alternate macro mode to allow more flexible syntax
# such as using `%expr` to evaluate the expression `expr` and use the result as a string.
# This is useful for generating labels dynamically.
# To disable alternate macro mode, use `.noaltmacro`.
.altmacro

### Define interrupt handlers for vectors 0 to __TOTAL_INTERRUPTS-1 here
.set i, 0
.rept (__TOTAL_INTERRUPTS)
    idt_interrupt_vector %i
    .set i, (i + 1)
.endr

.type idt_load, @function
idt_load:
    # Push caller's EBP to save the base pointer
    pushl %ebp
    movl  %esp, %ebp # Set up new base pointer for this function's stack frame

    # Get the input argument passed by the caller (pointer to IDT descriptor)
    movl  8(%ebp), %eax  # The first argument is at offset 8 from EBP (4 bytes for return address + 4 bytes for saved EBP)
    lidt  (%eax)         # Load the IDT using the pointer in EAX

    # Remove the stack frame created for this function
    movl  %ebp, %esp
    # Restore caller's EBP and return
    popl  %ebp
    ret

.type idt_enable_interrupts, @function
idt_enable_interrupts:
    sti  # Set Interrupt Flag to enable interrupts
    ret

.type idt_disable_interrupts, @function
idt_disable_interrupts:
    cli  # Clear Interrupt Flag to disable interrupts
    ret

.type idt_interrupt_stub, @function
idt_interrupt_stub:
    # This is a generic interrupt stub that can be used for unhandled interrupts.
    # It simply acknowledges the interrupt and returns.
    # Save caller's general-purpose registers
    pushal
    # Acknowledge the interrupt (send EOI to PIC)
    movb $0x20, %al
    outb %al, $0x20  # Send EOI to Master PIC
    # Restore caller's general-purpose registers
    popal
    iret  # Return from interrupt

.type idt_isr80h_handler_asm, @function
idt_isr80h_handler_asm: # void* (*c_handler)(int syscall_number, idt_interrupt_stack_frame_t* frame);
    # This is the wrapper for software interrupt 0x80 (system call)
    # The following registers have been pushed by the CPU automatically:
    # EIP, CS, EFLAGS, ESP, SS (if privilege level change)
    # We need to save the general-purpose registers as well.
    pushal  # Push all 32-bit general-purpose registers

    ### The end of interrupt frame is now set up. ###

    # Push the pointer to the interrupt frame onto the stack
    push %esp  # Push pointer to the interrupt frame

    # Push the syscall number (in EAX) onto the stack
    push %eax  # Push syscall number

    # Call the C handler for ISR 0x80
    call idt_isr80h_handler_c

    # Save the return value from the C handler in a temporary location
    movl %eax, temp_return_value
    # Clean up the stack (remove syscall number and frame pointer)
    addl $8, %esp

    # Restore all 32-bit general-purpose registers
    popal   # Pop all 32-bit general-purpose registers
    # Move the return value from the temporary location back into EAX
    movl temp_return_value, %eax
    iret    # Return from interrupt

.section .data
.align 4
temp_return_value: # Temporary storage for return value from C handler
    .long 0

### Define the interrupt handler table
.section .data
.align 4
idt_general_interrupt_handler_table:
    .set i, 0
    .rept __TOTAL_INTERRUPTS
        idt_interrupt_array_entry %i
        .set i, (i + 1)
    .endr
