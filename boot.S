.org 0x00
.code16
.section .text
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

    # BIOS disk read example
    # AH = 02h
    # AL = number of sectors to read (must be nonzero)
    # CH = low eight bits of cylinder number
    # CL = sector number 1-63 (bits 0-5)
    # high two bits of cylinder (bits 6-7, hard disk only)
    # DH = head number
    # DL = drive number (bit 7 set for hard disk) note: this is set by BIOS when it loads the boot sector
    # ES:BX -> data buffer
    movb    $0x02, %ah      # BIOS read sectors function
    movb    $0x01, %al      # Read 1 sector
    movb    $0x00, %ch      # Cylinder 0
    movb    $0x02, %cl      # Sector 2 (first sector is 1)
    movb    $0x00, %dh      # Head 0
    # DL is already set by BIOS to the boot drive
    movw    $buffer, %bx
    int     $0x13           # Call BIOS disk interrupt
    jc      .disk_error      # Jump if carry flag is set (error)

    movw    $buffer, %si  # Load address of buffer into SI
    # movw    $hello_msg, %si # Load address of hello_msg into SI
    call    print_hello
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

# the end of the bootloader code should be at offset 510
.org 510
.word 0xAA55            # Boot sector signature

# Reserve an address to store the additional message
buffer:
