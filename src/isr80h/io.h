#ifndef __ISR80H_IO_H__
#define __ISR80H_IO_H__

// Forward declaration
typedef struct idt_interrupt_stack_frame idt_interrupt_stack_frame_t;

void* io_isr80h_command_print(idt_interrupt_stack_frame_t* frame);
void* io_isr80h_command_get_keyboard_char(idt_interrupt_stack_frame_t* frame);
void* io_isr80h_command_put_char(idt_interrupt_stack_frame_t* frame);

#endif // __ISR80H_IO_H__