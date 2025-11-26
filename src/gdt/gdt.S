.code32
.section .asm

.global gdt_load
gdt_load:
    push %ebp           # Save base pointer
    mov  %esp, %ebp     # Set new base pointer

    mov  8(%esp), %eax  # First argument: address of GDT descriptor
    mov  %eax, gdt_descriptor+2  # Load address of GDT descriptor
    mov  12(%esp), %ax  # Second argument: limit of GDT descriptor
    mov  %ax, gdt_descriptor+0  # Store limit in GDT descriptor
    lgdt gdt_descriptor  # Load GDT

    mov  %ebp, %esp     # Restore stack pointer
    pop  %ebp           # Restore base pointer
    ret

.section .data
gdt_descriptor:
    .word 0               # Limit (size of GDT - 1)
    .long 0               # Base address of GDT