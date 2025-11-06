#include "config.h"

.code32

.section .text

.equ CODE_SEG, 0x08
.equ DATA_SEG, 0x10

# Only enable PIC1 (master PIC)
.equ PIC1_CMD, __PIC1_COMMAND_PORT
.equ PIC1_DATA, __PIC1_DATA_PORT
.equ PIC1_VECTOR_OFFSET, __PIC1_VECTOR_OFFSET

.extern kernel_main

.global _start

_start:
    # Initialize data segment registers
    movw    $DATA_SEG, %ax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %fs
    movw    %ax, %gs
    movw    %ax, %ss
    movl    $0x200000, %ebp  # Set stack pointer to 0x200000
    movl    %ebp, %esp

    # try to do 32-bit operations. The assigned value will be truncated to 16-bit when in real mode
    # movl    $0x12345678, %ebx

    # Remap the master PIC
    # ICW1: Initialize PIC
    movb    $0b00010011, %al # Edge triggered, single mode, ICW4 needed
    outb    %al, $PIC1_CMD

    # ICW2: Set vector offset
    movb    $PIC1_VECTOR_OFFSET, %al
    outb    %al, $PIC1_DATA

    # ICW4: Set 8086 mode
    movb    $0b00000001, %al # 8086 mode
    outb    %al, $PIC1_DATA

    # End of PIC remapping
    call    kernel_main

    jmp    . # Infinite loop

# padding to ensure minimum size
# Without padding, this kernel code will impact the alignment of the subsequent sections,
# and it may lead to unexpected behavior during linking or execution.
    .fill 512 - (. - _start), 1, 0
