#include "misc.h"
#include "task/task.h"

void* misc_isr80h_command_sum(idt_interrupt_stack_frame_t* frame) {
    // Extract the two integers from the stack frame
    int var1 = (int)task_get_stack_item(task_get_current(), 0);
    int var2 = (int)task_get_stack_item(task_get_current(), 1);
    return (void*)(var1 + var2);
}