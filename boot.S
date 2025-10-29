.org 0x00
.code16
.section .text
.equ CODE_SEG, gdt_kernel_code - gdt_start
.equ DATA_SEG, gdt_kernel_data - gdt_start
.global _start
# In case BIOS will tamper with first sector at where BPB(BIOS Parameter Block) is located,
# we deliberately reserve the space for the BPB before the actual code starts.
_start:
    # a short jump over BPB area
    jmp .code_start
    .org 0x21 # reserve space for BPB (33 bytes) excluding the jump instruction (3 bytes)

.code_start:
    cli                     # Clear interrupts
    xorw    %ax, %ax       # Zero out AX
    movw    %ax, %ds       # Set DS to 0
    movw    %ax, %es       # Set ES to 0
    movw    %ax, %ss       # Set SS to 0
    movw    $0x7C00, %sp   # Set stack pointer to 0x7C00
    sti                     # Enable interrupts

    # Enter protected mode
    cli                     # Clear interrupts
    lgdt    gdt_descriptor  # Load Global Descriptor Table
    movl    %cr0, %eax     # Get CR0
    orl     $0x1, %eax     # Set PE bit (bit 0)
    movl    %eax, %cr0     # Update CR0 to enter protected mode
    ljmp    $CODE_SEG, $.pm_start  # Far jump to protected mode code segment

    jmp     .               # Infinite loop

print_hello:
    mov $0x00, %bx        # Clear BX for BIOS teletype function
    .loop:
        lodsb              # Load byte at DS:SI into AL and increment SI
        cmp $0, %al        # Check for null terminator
        je .done
        call print_char     # Print character in AL
        jmp .loop
.done:
    ret

print_char:
    mov $0x0E, %ah        # BIOS teletype function
    int $0x10              # Call BIOS video interrupt
    ret

hello_msg:
    .asciz "Hello, World!"

.disk_error:
    movw    $error_msg, %si  # Load address of error_msg into SI
    call    print_hello
    jmp     .

error_msg:
    .asciz "Disk Read Error!"

# Global Descriptor Table (GDT) setup for protected mode
gdt_start:
gdt_null:
    .quad 0x0000000000000000  # Null descriptor, the first entry must be null
gdt_kernel_code:
    .word 0xFFFF              # Limit low (0:15 bit), set to maximum (4 GB)
    .word 0x0000              # Base low (0:15 bit)
    .byte 0x00                # Base middle (16:23 bit)
    .byte 0x9A                # Access byte: present, ring 0, code segment, executable, readable
    .byte 0xCF                # Granularity: 4K granularity, 32-bit (0xC, 0b1100). Limit high: 0xF (0b1111)(16:19 bit)
    .byte 0x00                # Base high
gdt_kernel_data:
    .word 0xFFFF              # Limit low (0:15 bit), set to maximum (4 GB)
    .word 0x0000              # Base low (0:15 bit)
    .byte 0x00                # Base middle (16:23 bit)
    .byte 0x92                # Access byte: present, ring 0, data segment, writable
    .byte 0xCF                # Granularity: 4K granularity, 32-bit (0xC, 0b1100). Limit high: 0xF (0b1111)(16:19 bit)
    .byte 0x00                # Base high
gdt_end:
gdt_descriptor:
    .word gdt_end - gdt_start - 1  # Limit (max offset from base address)
    .long gdt_start                # Base address

# Protected mode code segment, 32-bit
.code32
.pm_start:
    # Initialize data segment registers
    movw    $DATA_SEG, %ax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %fs
    movw    %ax, %gs
    movw    %ax, %ss
    # move stack to higher address to validate we are in protected mode
    movl    $0x200000, %esp
    movl    %esp, %ebp
    jmp    . # Infinite loop

# the end of the bootloader code should be at offset 510
.org 510
.word 0xAA55            # Boot sector signature

# Reserve an address to store the additional message
buffer: .space 512
