#include "heap.h"
#include "task/task.h"
#include "task/process.h"
#include <stddef.h>

/**
 * @brief Handle the ISR 0x80 command for memory allocation (malloc).
 * @param frame Pointer to the interrupt stack frame.
 * @return Pointer to the allocated memory, or NULL on failure.
 */
void* heap_isr80h_command_malloc(idt_interrupt_stack_frame_t* frame) {
    // Get current task
    task_t* current_task = task_get_current();
    if (!current_task) {
        return NULL; // No current task, cannot allocate
    }

    // The size to allocate is expected to be in task's stack
    size_t size = (size_t)task_get_stack_item(current_task, 0);
    if (size == 0) {
        return NULL; // Cannot allocate 0 bytes
    }

    // Allocate memory for the current process
    return process_malloc(current_task->process, size);
}

/**
 * @brief Handle the ISR 0x80 command for freeing memory (free).
 * @param frame Pointer to the interrupt stack frame.
 * @return NULL.
 */
void* heap_isr80h_command_free(idt_interrupt_stack_frame_t* frame) {
    // Get current task
    task_t* current_task = task_get_current();
    if (!current_task) {
        return NULL; // No current task, cannot free
    }

    // The pointer to free is expected to be in task's stack
    void* ptr = (void*)task_get_stack_item(current_task, 0);
    if (!ptr) {
        return NULL; // Cannot free NULL pointer
    }

    // Free memory for the current process
    process_free(current_task->process, ptr);
    return NULL;
}