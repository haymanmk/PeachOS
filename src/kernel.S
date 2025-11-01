.code32

.equ CODE_SEG, 0x08
.equ DATA_SEG, 0x10

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
    movl    $0x12345678, %ebx

    call    kernel_main

    jmp    . # Infinite loop

# padding to ensure minimum size
# Without padding, this kernel code will impact the alignment of the subsequent sections,
# and it may lead to unexpected behavior during linking or execution.
    .fill 512 - (. - _start), 1, 0
