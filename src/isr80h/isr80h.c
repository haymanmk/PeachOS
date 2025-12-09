#include "isr80h.h"
#include "idt/idt.h"
#include "misc.h"
#include "status.h"
#include "io.h"

/**
 * @brief Register ISR 0x80 command handlers.
 * @return 0 on success, negative error code on failure.
 */
int isr80h_register_commands() {
    int res = 0;
    res += idt_isr80h_register_handler(ISR80H_CMD_SUM, misc_isr80h_command_sum);
    res += idt_isr80h_register_handler(ISR80H_CMD_PRINT, io_isr80h_command_print);

    return res;
}