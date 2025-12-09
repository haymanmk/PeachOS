#ifndef __ISR80H_H__
#define __ISR80H_H__

/**
 * @brief Command numbers for ISR 0x80 system calls.
 */
typedef enum {
    ISR80H_CMD_SUM,
    ISR80H_CMD_PRINT,
} isr80h_command_num_t;

int isr80h_register_commands();

#endif // __ISR80H_H__