#ifndef __MISC_H__
#define __MISC_H__

#include "idt/idt.h"

// Forward declarations to avoid circular dependencies
typedef struct idt_interrupt_stack_frame idt_interrupt_stack_frame_t;

void* misc_isr80h_command_sum(idt_interrupt_stack_frame_t* frame);

#endif // __MISC_H__