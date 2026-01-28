#ifndef __HEAP_ISR80H_H__
#define __HEAP_ISR80H_H__

#include "idt/idt.h"

void* heap_isr80h_command_malloc(idt_interrupt_stack_frame_t* frame);

#endif // __HEAP_ISR80H_H__