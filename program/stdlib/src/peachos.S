.code32

#include "isr80h/isr80h.h"

.section .asm
.global peachos_print
.global peachos_getchar
.global peachos_putchar
.global peachos_malloc
.global peachos_free

# int peachos_print(const char* str)
.type peachos_print, @ function
peachos_print:
    push %ebp               # Save base pointer
    mov  %esp, %ebp         # Set up new base pointer
    # Push the string argument onto the stack for the syscall
    push 8(%ebp)            # First argument is at offset 8 from EBP
    mov $ISR80H_CMD_PRINT, %eax   # Syscall number for print
    int $ISR80H_INTERRUPT_NUMBER             # Trigger software interrupt, ISR 0x80 for syscall
    add  $4, %esp          # Clean up stack after call
    pop  %ebp               # Restore base pointer
    ret

# char peachos_getchar()
# Get keyboard character from ISR80H
.type peachos_getchar, @ function
peachos_getchar:
    push %ebp               # Save base pointer
    mov  %esp, %ebp         # Set up new base pointer
    mov $ISR80H_CMD_GET_KEYBOARD_CHAR, %eax   # Syscall number for getchar
    int $ISR80H_INTERRUPT_NUMBER             # Trigger software interrupt, ISR 0x80 for syscall
    pop  %ebp               # Restore base pointer
    ret

# void peachos_putchar(char c)
.type peachos_putchar, @ function
peachos_putchar:
    push %ebp               # Save base pointer
    mov  %esp, %ebp         # Set up new base pointer
    push 8(%ebp)            # Push character argument
    mov $ISR80H_CMD_PUTCHAR, %eax   # Syscall number for putchar
    int $ISR80H_INTERRUPT_NUMBER             # Trigger software interrupt, ISR 0x80 for syscall
    add  $4, %esp          # Clean up stack after call. i.e. Remove character argument
    pop  %ebp               # Restore base pointer
    ret

# void* peachos_malloc(size_t size)
.type peachos_malloc, @ function
peachos_malloc:
    push %ebp               # Save base pointer
    mov  %esp, %ebp         # Set up new base pointer
    push 8(%ebp)            # Push size argument
    mov $ISR80H_CMD_MALLOC, %eax   # Syscall number for malloc
    int $ISR80H_INTERRUPT_NUMBER             # Trigger software interrupt, ISR 0x80 for syscall
    add  $4, %esp          # Clean up stack after call. i.e. Remove size argument
    pop  %ebp               # Restore base pointer
    ret

# void peachos_free(void* ptr)
.type peachos_free, @ function
peachos_free:
    push %ebp               # Save base pointer
    mov  %esp, %ebp         # Set up new base pointer
    push 8(%ebp)            # Push pointer argument
    mov $ISR80H_CMD_FREE, %eax   # Syscall number for free
    int $ISR80H_INTERRUPT_NUMBER             # Trigger software interrupt, ISR 0x80 for syscall
    add  $4, %esp          # Clean up stack after call. i.e. Remove pointer argument
    pop  %ebp               # Restore base pointer
    ret