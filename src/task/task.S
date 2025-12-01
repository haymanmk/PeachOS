.code32
.section .asm

# Import constants from config.h
#include "config.h"

.equ __USER_DATA_SELECTOR, USER_DATA_SELECTOR
.equ __RPL_USER, RPL_USER

.global task_return_to_user_mode
.global task_restore_general_registers
.global task_restore_user_data_segment

.type task_return_to_user_mode, @ function
task_return_to_user_mode: # void task_return_to_user_mode(task_registers_t* registers)
    push %ebp               # Save base pointer
    mov  %esp, %ebp         # Set up new base pointer
    
    # Push user data onto the stack for iret
    # push user data segment selector (registers->ss)
    # push user stack pointer (registers->user_esp)
    # push EFLAGS (registers->eflags)
    # push user code segment selector (registers->cs)
    # push user instruction pointer (registers->eip)

    # Get the pointer to task_registers_t from the stack
    mov  8(%ebp), %ebx      # First argument is at offset 8 from EBP which is a pointer to task_registers_t
    push 44(%ebx)      # Push registers->ss
    push 40(%ebx)      # Push registers->user_esp

    # Push current EFLAGS and set IF (Interrupt Flag)
    pushf
    pop  %eax
    or   $0x200, %eax        # Set IF flag
    push %eax                 # Push modified EFLAGS

    push 32(%ebx)      # Push registers->cs
    push 28(%ebx)      # Push registers->eip

    # Setup segment registers
    mov 44(%ebx), %ax   # Load user data segment selector into AX
    mov  %ax, %ds
    mov  %ax, %es
    mov  %ax, %fs
    mov  %ax, %gs

    # Restore general purpose registers
    push %ebx               # Pass pointer to task_registers_t as argument
    call task_restore_general_registers
    add  $4, %esp          # Clean up stack after call

    # Leave kernel land and return to user mode
    iretl

.type task_restore_general_registers, @ function
task_restore_general_registers: # void task_restore_general_registers(task_registers_t* registers
    # push %ebp               # Save base pointer
    # mov  %esp, %ebp         # Set up new base pointer

    # Get the pointer to task_registers_t from the stack
    mov  4(%ebp), %ebx      # First argument is at offset 8 from EBP which is a pointer to task_registers_t
    mov  0(%ebx), %edi      # Load registers->edi
    mov  4(%ebx), %esi      # Load registers->esi
    mov  8(%ebx), %ebp      # Load registers->ebp
    mov 16(%ebx), %edx      # Load registers->edx
    mov 20(%ebx), %ecx      # Load registers->ecx
    mov 24(%ebx), %eax      # Load registers->eax
    mov 12(%ebx), %ebx      # Load registers->ebx

    # The following lines will mess up the restore stack frame, so we comment them out.
    # Restore base pointer and return
    # mov  %ebp, %esp         # Restore stack pointer
    # pop  %ebp                # Restore base pointer

    ret

.type task_restore_user_data_segment, @ function
task_restore_user_data_segment: # void task_restore_user_data_segment()
    push %ebp               # Save base pointer
    mov  %esp, %ebp         # Set up new base pointer

    mov  $__USER_DATA_SELECTOR | __RPL_USER, %ax  # Load user data segment selector with RPL
    mov  %ax, %ds
    mov  %ax, %es
    mov  %ax, %fs
    mov  %ax, %gs

    mov  %ebp, %esp         # Restore stack pointer
    pop  %ebp                # Restore base pointer

    ret