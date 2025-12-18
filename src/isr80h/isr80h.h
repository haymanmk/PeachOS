#ifndef __ISR80H_H__
#define __ISR80H_H__

#include "idt/idt.h"

typedef void*(*isr80h_command_handler_t)(idt_interrupt_stack_frame_t* frame);

/**
 * @brief Command numbers for ISR 0x80 system calls.
 */
typedef enum {
    ISR80H_CMD_SUM,
    ISR80H_CMD_PRINT,
    ISR80H_CMD_GET_KEYBOARD_CHAR,
    ISR80H_CMD_PUT_CHAR, // to terminal
} isr80h_command_num_t;

int isr80h_register_commands();
int isr80h_register_handler(int command_number, idt_interrupt_handler_t handler);
void* isr80h_handler_c(int syscall_number, idt_interrupt_stack_frame_t* frame);
void* isr80h_handle_command(int command_number, idt_interrupt_stack_frame_t* frame);

#endif // __ISR80H_H__