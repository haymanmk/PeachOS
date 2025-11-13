.org 0x00
.code16
.section .text
.equ CODE_SEG, gdt_kernel_code - gdt_start
.equ DATA_SEG, gdt_kernel_data - gdt_start
.global _start
# In case BIOS will tamper with first sector at where BPB(BIOS Parameter Block) is located,
# we deliberately reserve the space for the BPB before the actual code starts.
_start:
    # A short jump over BPB area which occupies the first 3 bytes of BPB
    jmp .code_start
    nop                     # Padding to make up 3 bytes

    # Define the rest of the BPB headers here (FAT16)
    # Note: the type of FAT (FAT12, FAT16, FAT32) is determined by the number of clusters in the data region.
    #       If the number of clusters is less than 4085, it's FAT12; if it's between 4085 and 65525, it's FAT16; otherwise, it's FAT32.
    OEMIdentifier:        .ascii "PEACHOS "   # 8 bytes
    BytesPerSector:       .word 512           # 512 = 0x200
    SectorsPerCluster:    .byte 0x80          # Cluster is where file data is stored, 0x80 = 128 sectors per cluster
    ReservedSectors:      .word 200           # Here we reserve 200 sectors for bootloader and our kernel
    NumberOfFATs:         .byte 2             # Should be 2 in case the compatibility issue
    MaxRootDirEntries:    .word 0x40          # The maximum number of 32-byte directory entries in the root directory
    TotalSectors16:       .word 0             # Total sectors used in old 16-bit field (if zero, use TotalSectors32)
    MediaDescriptor:      .byte 0xF8          # Type of media, 0xF8 for fixed disk (non-removable)
    SectorsPerFAT16:      .word 0x100         # Number of sectors per FAT (FAT12/16)
    SectorsPerTrack:      .word 0x10          # Number of sectors per track
    NumberOfHeads:        .word 0x40          # Number of heads
    HiddenSectors:        .long 0             # Number of hidden sectors preceding FAT volume. 0 if the volume starts at the beginning of the disk
    TotalSectors32:       .long 0x773594      # Total sectors used in 32-bit field (if TotalSectors16 is zero)

    # Extended BPB can go here (we use FAT16)
    DriveNumber:         .byte 0x80           # For hard disks (fixed disk), 0x80
    Reserved1:           .byte 0              # 1 byte
    BootSignature:       .byte 0x29           # This indicates that the next three fields are present
    VolumeID:            .long 0xD105         # Serial number of the volume (you can set it to any value)
    VolumeLabel:         .ascii "PEACHOSBOOT" # 11 bytes. Volume label in ASCII
    FileSystemType:      .ascii "FAT16   "    # 8 bytes. File system type in ASCII

    # Bootstrap code (Bootloader) starts here. Total size should be 448 bytes and filled with 0 if not used.
.code_start:
    cli                     # Clear interrupts
    xorw    %ax, %ax       # Zero out AX
    movw    %ax, %ds       # Set DS to 0
    movw    %ax, %es       # Set ES to 0
    movw    %ax, %ss       # Set SS to 0
    movw    $0x7C00, %sp   # Set stack pointer to 0x7C00
    sti                     # Enable interrupts

    # Print a message indicating the bootloader is in protected mode
    movw    $message_entering_pm, %si   # Use %si (16-bit) in real mode
    call    print_message

    # Enter protected mode
    cli                     # Clear interrupts

    # Enable A20 line
    inb     $0x92, %al
    orb     $0x02, %al
    outb    %al, $0x92

    lgdt    gdt_descriptor  # Load Global Descriptor Table
    movl    %cr0, %eax     # Get CR0
    orl     $0x1, %eax     # Set PE bit (bit 0)
    movl    %eax, %cr0     # Update CR0 to enter protected mode

    ljmp    $CODE_SEG, $pm_start  # Far jump to flush the prefetch intruction and load CS from GDT in protected mode
    # jmp     pm_start  # short jump, CS is still 0 in real mode
    jmp     .               # Infinite loop

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

# Data section for messages
message_entering_pm:
    .asciz "Entering Protected Mode!"

# @brief Print a null-terminated string to the screen
# @param: %si (real mode, 16-bit) or %esi (protected mode, 32-bit) - pointer to the string
print_message:
    xorw    %bx, %bx        # Clear BX before using teletype function
.loop:
    lodsb                     # Load byte at DS:(E)SI into AL, increment (E)SI
    cmpb    $0, %al           # Check for null terminator
    je      .done             # If zero, we're done
    call    print_char        # Print the character in AL
    jmp     .loop             # Repeat for next character
.done:
    ret

# @brief Print a character to the screen by using BIOS interrupt 0x10
# @param: %al - character to print
print_char:
    movb   $0x0E, %ah        # BIOS teletype function
    int    $0x10             # Call BIOS interrupt
    ret

# Protected mode code segment, 32-bit
.code32
pm_start:
    # read kernel from disk
    movl    $0x01, %eax        # LBA of the first sector to read (sector 1)
    movl    $100, %ecx       # number of sectors to read
    movl    $0x00100000, %edi  # destination memory address (1 MB)
    call    ata_lba_read

    jmp     $CODE_SEG, $0x00100000  # Jump to the loaded kernel at 1 MB

# @param: %eax - LBA of the first sector to read
#         %ecx - number of sectors to read
#         %edi - destination memory address
ata_lba_read:
    movl    %eax, %ebx    # Backup LBA to EBX

    # Set drive/head register
    # Set the LBA bits 24-27 and set bit 6 (LBA mode) and bit 4 (master drive)
    movw   $0x01F6, %dx  # 0x1F6 is the I/O port for drive/head register
    shr    $24, %eax
    orb    $0xE0, %al      # 0xE0 = 11100000b
    outb   %al, %dx

    # Set sector count
    movw   $0x01F2, %dx  # 0x1F2 is the I/O port for sector count
    movb   %cl, %al    # Number of sectors to read
    outb   %al, %dx

    # Set LBA low bytes (0-23)
    movw   $0x01F3, %dx  # 0x1F3 is the I/O port for LBA low byte
    movb   %bl, %al    # LBA bits 0-7
    outb   %al, %dx
    movw   $0x01F4, %dx  # 0x1F4 is the I/O port for LBA mid byte
    shr    $8, %ebx
    movb   %bl, %al    # LBA bits 8-15
    outb   %al, %dx
    movw   $0x01F5, %dx  # 0x1F5 is the I/O port for LBA high byte
    shr    $16, %ebx
    movb   %bl, %al    # LBA bits 16-23
    outb   %al, %dx

    # Send read command
    movw   $0x01F7, %dx  # 0x1F7 is the I/O port for command register
    movb   $0x20, %al  # 0x20 is the READ SECTORS command with retries
    outb   %al, %dx

.read_loop:
    push   %ecx            # Save sector count
.waiting:
    # Wait for the drive to be ready
    movw   $0x01F7, %dx
    in     %dx, %al
    testb  $0x08, %al      # Check the DRQ (Data Request) bit
    jz     .waiting       # If not ready, keep waiting

    # Read a sector (512 bytes)
    movw   $256, %cx      # 256 words = 512 bytes
    movw   $0x01F0, %dx  # 0x1F0 is the data port
    rep    insw            # Read words from port to memory at ES:DI
    pop    %ecx            # Restore sector count
    loop   .read_loop      # Decrement ECX and loop if not zero

    ret


# The end of the bootloader code should be at offset 510
.org 510
.word 0xAA55            # Boot sector signature
