#include "isr80h.h"
#include "misc.h"
#include "status.h"
#include "io.h"
#include "config.h"
#include "kernel.h"
#include "task/task.h"
#include <stddef.h>

static isr80h_command_handler_t isr80h_command_handlers[ISR80H_MAX_COMMANDS];

/**
 * @brief Register ISR 0x80 command handlers.
 * @return 0 on success, negative error code on failure.
 */
int isr80h_register_commands() {
    int res = 0;
    res += isr80h_register_handler(ISR80H_CMD_SUM, misc_isr80h_command_sum);
    res += isr80h_register_handler(ISR80H_CMD_PRINT, io_isr80h_command_print);
    res += isr80h_register_handler(ISR80H_CMD_GET_KEYBOARD_CHAR, io_isr80h_command_get_keyboard_char);
    res += isr80h_register_handler(ISR80H_CMD_PUT_CHAR, io_isr80h_command_put_char);

    return res;
}

/**
 * @brief Register a handler for a specific system call command number.
 * @param command_number The system call command number.
 * @param handler The handler function to register.
 * @return 0 on success, negative error code on failure.
 * @note This function can be used in files like isr80h.c to register command handlers.
 */
int isr80h_register_handler(int command_number, idt_interrupt_handler_t handler) {
    if (command_number < 0 || command_number >= ISR80H_MAX_COMMANDS) {
        return -EINVAL; // Invalid command number
    }
    isr80h_command_handlers[command_number] = handler;
    return ENONE;
}

/**
 * @brief The ISR 0x80 handler in C for system calls.
 * @param syscall_number The system call number. (mapped to commands or services)
 * @param frame Pointer to the interrupt stack frame.
 * @return A void pointer (can be used to return values if needed).
 */
void* isr80h_handler_c(int syscall_number, idt_interrupt_stack_frame_t* frame) {
    // Handle system call based on syscall_number
    void* return_value = NULL;

    // switch to kernel paging
    kernel_page();

    // Save the current task's state such as registers
    task_save_current_state(frame);

    // Process the system call
    return_value = isr80h_handle_command(syscall_number, frame);

    // Retrun to user pageing after syscall handling
    task_page_current();

    return return_value;
}

/**
 * @brief Retrieve and invoke the registered handler for a given system call command number.
 * @param syscall_number The system call command number.
 * @param frame Pointer to the interrupt stack frame.
 * @return The return value from the invoked handler, or NULL if no handler is registered.
 */
void* isr80h_handle_command(int syscall_number, idt_interrupt_stack_frame_t* frame) {
    if (syscall_number < 0 || syscall_number >= ISR80H_MAX_COMMANDS) {
        return NULL; // Invalid command number
    }

    isr80h_command_handler_t handler = isr80h_command_handlers[syscall_number];
    if (handler) {
        return handler(frame);
    }
    return NULL; // No handler registered for this command
}