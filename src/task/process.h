#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "task.h"
#include "config.h"
#include <stdint.h>

typedef struct task task_t; // Forward declaration

typedef struct process {
    // Define process-related fields here
    uint16_t pid; // Process ID
    char filename[256]; // Executable filename
    task_t* main_task; // Pointer to the main task of the process
    void* mem_alloc[PROGRAM_MAX_ALLOCATIONS]; // Track memory allocations (malloc) which need to be freed on process termination
    void* file_ptr; // File pointer to the executable file
    uint32_t file_size; // Size of the executable file
    void* stack; // Pointer to the process's stack

    // Keyboard ring buffer to store keyboard input for this process
    struct keyboard_buffer {
        char buffer[KEYBOARD_BUFFER_SIZE];
        uint32_t head;
        uint32_t tail;
    } keyboard;
} process_t;

int process_load(const char* filename, process_t** out_process);
int process_load_into_slot(const char* filename, process_t** out_process, uint16_t process_slot);
process_t* process_get_current();
process_t* process_get_by_pid(uint16_t pid);

#endif // __PROCESS_H__