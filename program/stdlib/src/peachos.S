.code32

#include "isr80h/isr80h.h"

.equ SYS_PRINT, ISR80H_CMD_PRINT
.equ ISR80H, ISR80H_INTERRUPT_NUMBER

.section .asm
.global print

# void print(const char* str)
.type print, @ function
print:
    push %ebp               # Save base pointer
    mov  %esp, %ebp         # Set up new base pointer

    # Push the string argument onto the stack for the syscall
    mov  8(%ebp), %eax      # First argument is at offset 8 from EBP
    push %eax                # Push string argument
    mov $SYS_PRINT, %eax   # Syscall number for print
    int $ISR80H             # Trigger software interrupt, ISR 0x80 for syscall
    add  $4, %esp          # Clean up stack after call
    pop  %ebp               # Restore base pointer
    ret