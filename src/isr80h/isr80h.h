#ifndef __ISR80H_H__
#define __ISR80H_H__

/**
 * @brief Command numbers for ISR 0x80 system calls.
 */
#define ISR80H_CMD_SUM 0
#define ISR80H_CMD_PRINT 1
#define ISR80H_CMD_GET_KEYBOARD_CHAR 2
#define ISR80H_CMD_PUT_CHAR 3

#define ISR80H_INTERRUPT_NUMBER 0x80

/**
 * @note The following definitions are only for C code.
 *       They will be excluded when __ASSEMBLER__ is defined.
 *       This is to prevent issues when including this header in assembly files.
 *       Fortunately, assembler defines __ASSEMBLER__ by default.
 */
#ifndef __ASSEMBLER__

#include "idt/idt.h"

typedef uint16_t isr80h_command_num_t;
typedef void*(*isr80h_command_handler_t)(idt_interrupt_stack_frame_t* frame);

int isr80h_register_commands();
int isr80h_register_handler(int command_number, idt_interrupt_handler_t handler);
void* isr80h_handler_c(int syscall_number, idt_interrupt_stack_frame_t* frame);
void* isr80h_handle_command(int command_number, idt_interrupt_stack_frame_t* frame);

#endif // __ASSEMBLER__

#endif // __ISR80H_H__