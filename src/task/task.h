#ifndef __TASK_H__
#define __TASK_H__

#include "config.h"
#include "process.h"
#include "memory/paging/paging.h"
#include "idt/idt.h"

typedef struct process process_t; // Forward declaration

// Registers saved during a context switch
typedef struct {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t user_esp;
    uint32_t ss;
} task_registers_t;

// Task Control Block (TCB) structure
typedef struct task {
    uint32_t pid; // Process ID
    paging_4gb_chunk_t* paging_chunk; // Pointer to the process's paging chunk
    // Add more fields as necessary (e.g., CPU registers, state, etc.)
    task_registers_t registers;
    process_t* process; // Pointer to the associated process
    struct task* next; // Pointer to the next task in the list
    struct task* prev; // Pointer to the previous task in the list
} task_t;

task_t* task_new(process_t* process);
int task_free(task_t* task);
task_t* task_get_current();
task_t* task_get_next();
int task_switch(task_t* next_task);
int task_page_current();
int task_page_task(task_t* task);
void task_run_first_ever_task();
void task_save_current_state(idt_interrupt_stack_frame_t* frame);
int task_copy_string_from_task(task_t* task, const char* src_virt_addr, char* dest_phys_addr, size_t max_length);
void* task_get_stack_item(task_t* task, uint32_t index);

/**
 * Returns to user mode from kernel mode or from an interrupt.
 * This function is implemented in assembly.
 * It is responsible for restoring the user mode stack and
 * executing the IRET instruction to switch to user mode.
 */
void task_return_to_user_mode(task_registers_t* registers);
/**
 * Restores the general-purpose registers from the given task_registers_t structure.
 * This function is implemented in assembly.
 */
void task_restore_general_registers(task_registers_t* registers);
/**
 * Restores the user segment selector registers (DS, ES, FS, GS) for user mode
 * by pointing them to the user data segment. The data segment selector is fixed
 * as USER_DATA_SELECTOR in config.h.
 * This function is implemented in assembly.
 */
void task_restore_user_data_segment();

#endif // __TASK_H__