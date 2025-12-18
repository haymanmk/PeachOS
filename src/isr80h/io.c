#include "io.h"
#include "task/task.h"
#include "utils/stdio.h"
#include "status.h"
#include "keyboard/keyboard.h"

#define MAX_PRINT_LENGTH 1024

/**
 * @brief Handle the print command from ISR 0x80.
 * @param frame Pointer to the interrupt stack frame.
 * @return ENONE on success, negative error code on failure.
 */
void* io_isr80h_command_print(idt_interrupt_stack_frame_t* frame) {
    //////////////////////////////////////
    // We are in kernel mode here
    //////////////////////////////////////
    int res = ENONE;
    if (!frame) {
        res = -EINVAL; // Invalid argument
        goto exit;
    }
    task_t* current_task = task_get_current();
    if (!current_task) {
        res = -EFAULT; // No current task
        goto exit;
    }
    // Get the pointer to the string from the stack
    const char* str_ptr = (const char*)task_get_stack_item(current_task, 0);
    if (!str_ptr) {
        res = -EFAULT; // Failed to get string pointer
        goto exit;
    }
    // Copy the string from the task's memory space to a kernel buffer
    char buffer[MAX_PRINT_LENGTH];
    res = task_copy_string_from_task(current_task, str_ptr, buffer, MAX_PRINT_LENGTH);
    if (res != ENONE) {
        goto exit; // Propagate error
    }
    // Print the string to the console
    printf("%s", buffer);

exit:
    return ERROR_VOID(res);
}

/**
 * @brief Handle the get keyboard character (scan code) command from ISR 0x80.
 * @param frame Pointer to the interrupt stack frame.
 * @return The character retrieved from the keyboard buffer.
 */
void* io_isr80h_command_get_keyboard_char(idt_interrupt_stack_frame_t* frame) {
    //////////////////////////////////////
    // We are in kernel mode here
    //////////////////////////////////////
    char c = keyboard_pop();
    return (void*)(intptr_t)c;
}

/**
 * @brief Handle the put character (at terminal) command from ISR 0x80.
 * @param frame Pointer to the interrupt stack frame.
 * @return ENONE on success, negative error code on failure.
 */
void* io_isr80h_command_put_char(idt_interrupt_stack_frame_t* frame) {
    //////////////////////////////////////
    // We are in kernel mode here
    //////////////////////////////////////
    // Get the character from the stack
    task_t* current_task = task_get_current();
    if (!current_task) {
        return ERROR_VOID(-EFAULT); // No current task
    }
    char c = (char)(intptr_t)task_get_stack_item(current_task, 0);
    // Output the character to the terminal
    printf("%c", c);
    return ERROR_VOID(ENONE);
}