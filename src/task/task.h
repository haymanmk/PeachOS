#ifndef __TASK_H__
#define __TASK_H__

#include "config.h"
#include "memory/paging/paging.h"

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
typedef struct tcb {
    uint32_t pid; // Process ID
    paging_4gb_chunk_t* paging_chunk; // Pointer to the process's paging chunk
    // Add more fields as necessary (e.g., CPU registers, state, etc.)
    task_registers_t registers;
    struct tcb* next; // Pointer to the next task in the list
    struct tcb* prev; // Pointer to the previous task in the list
} tcb_t;

tcb_t* task_new();
tcb_t* task_get_current();
tcb_t* task_get_next();
int task_switch(tcb_t* next_task);

#endif // __TASK_H__