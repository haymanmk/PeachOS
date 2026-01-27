.code32

#include "isr80h/isr80h.h"

.section .asm
.global print
.global getchar

# void print(const char* str)
.type print, @ function
print:
    push %ebp               # Save base pointer
    mov  %esp, %ebp         # Set up new base pointer

    # Push the string argument onto the stack for the syscall
    mov  8(%ebp), %eax      # First argument is at offset 8 from EBP
    push %eax                # Push string argument
    mov $ISR80H_CMD_PRINT, %eax   # Syscall number for print
    int $ISR80H_INTERRUPT_NUMBER             # Trigger software interrupt, ISR 0x80 for syscall
    add  $4, %esp          # Clean up stack after call
    pop  %ebp               # Restore base pointer
    ret

# char getchar()
# Get keyboard character from ISR80H
.type getchar, @ function
getchar:
    push %ebp               # Save base pointer
    mov  %esp, %ebp         # Set up new base pointer
    mov $ISR80H_CMD_GET_KEYBOARD_CHAR, %eax   # Syscall number for getchar
    int $ISR80H_INTERRUPT_NUMBER             # Trigger software interrupt, ISR 0x80 for syscall
    pop  %ebp               # Restore base pointer
    ret