# Interrupt Descriptor Table (IDT) management
.code32
.section .asm, "ax", @progbits  # Define .asm section as executable code

## Only for TEST: INT21h handler written in C.
.extern int21h_handler_c
.global int21h_handler_asm
.type int21h_handler_asm, @function
int21h_handler_asm:
    # clear interrupt flag to prevent nested interrupts
    cli
    # Push caller's 32-bit registers to save their state
    pushl %eax
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %esi
    pushl %edi
    pushl %ebp
    
    call int21h_handler_c
    # Restore caller's 32-bit registers
    popl %ebp
    popl %edi
    popl %esi
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax
    # set interrupt flag to re-enable interrupts
    sti
    iret
### Only for TEST END


.global idt_load
.global idt_enable_interrupts
.global idt_disable_interrupts
.global idt_interrupt_stub

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
    pushl %eax
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %esi
    pushl %edi
    pushl %ebp
    # Acknowledge the interrupt (send EOI to PIC)
    movb $0x20, %al
    outb %al, $0x20  # Send EOI to Master PIC
    # Restore caller's general-purpose registers
    popl %ebp
    popl %edi
    popl %esi
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax
    iret  # Return from interrupt
