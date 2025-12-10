#include "process.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "status.h"
#include "task/task.h"
#include "utils/string.h"
#include "fs/file.h"

process_t* current_process = NULL; // Pointer to the currently running process
static process_t* process_table[PROGRAM_MAX_PROCESSES]; // Fixed-size process table

/**
 * @brief Retrieve a process by its slot index.
 * @param slot The slot index of the process.
 * @return Pointer to the process, or NULL if the slot is invalid or empty.
 */
process_t* process_get_process_by_slot(uint16_t slot) {
    if (slot >= PROGRAM_MAX_PROCESSES) {
        return NULL;
    }
    return process_table[slot];
}

/**
 * @brief Load a binary executable file into memory.
 * @param filename The path to the executable file.
 * @param process Pointer to the process structure.
 * @return ENONE on success, negative error code on failure.
 */
int process_load_binary(const char* filename, process_t* process) {
    int res = 0;
    int fd = file_open(filename, "r");
    if (fd < 0) {
        return fd; // Propagate error code
    }

    // Read the file status
    file_state_t file_state;
    res = file_stat(fd, &file_state);
    if (res < 0) {
        goto exit;
    }

    // Allocate memory for the executable
    process->file_size = file_state.file_size;
    process->file_ptr = kheap_zmalloc(process->file_size);
    if (!process->file_ptr) {
        res = -ENOMEM;
        goto exit;
    }

    // Read the executable into memory
    size_t total_read = file_read(process->file_ptr, process->file_size, 1, fd);
    if (total_read != process->file_size) {
        res = -EIO;
        goto exit;
    }

exit:
    file_close(fd);
    return res;
}

/**
 * @brief Map the process's physical memory into its virtual memory space.
 * @param process Pointer to the process structure.
 * @return ENONE on success, negative error code on failure.
 */
int process_map_memory(process_t* process) {
    int res = 0;
    // Map the binary to the predefined virtual address
    res = paging_map_virtual_addresses(
        process->main_task->paging_chunk,
        PROGRAM_VIRTUAL_ADDRESS,
        (uint32_t)process->file_ptr,
        process->file_size,
        PAGING_FLAG_PRESENT | PAGING_FLAG_USER | PAGING_FLAG_WRITABLE
    );
    if (res < 0) {
        goto exit;
    }

    // Map the stack to the predefined virtual stack address.
    // Stack grows downwards, so we map from bottom to top.
    // After mapping, the C program can retrieve the arguments from the stack.
    // The processor will set the stack pointer (ESP) to the top of the stack
    // when switching to user mode.
    res = paging_map_virtual_addresses(
        process->main_task->paging_chunk,
        PROGRAM_VIRTUAL_STACK_BOTTOM_ADDRESS,
        (uint32_t)process->stack,
        PROGRAM_VIRTUAL_STACK_SIZE_BYTES,
        PAGING_FLAG_PRESENT | PAGING_FLAG_USER | PAGING_FLAG_WRITABLE
    );
    if (res < 0) {
        goto exit;
    }

exit:
    return res;
}

/**
 * @brief Find a free slot in the process table.
 * @return The index of a free slot, or -EBUSY if no slots are available.
 */
int process_get_free_slot() {
    for (uint16_t i = 0; i < PROGRAM_MAX_PROCESSES; i++) {
        if (process_table[i] == NULL) {
            return i;
        }
    }

    return -EBUSY; // No free slots available
}

/**
 * @brief Load a process from an executable file.
 * @param filename The path to the executable file.
 * @param out_process Pointer to store the created process.
 * @return ENONE on success, negative error code on failure.
 */
int process_load(const char* filename, process_t** out_process) {
    int slot = process_get_free_slot();
    if (slot < 0) {
        return slot; // Propagate error code
    }

    return process_load_into_slot(filename, out_process, (uint16_t)slot);
}

/**
 * @brief Load a process into a specified slot.
 * @param filename The path to the executable file.
 * @param out_process Pointer to store the created process.
 * @param process_slot The slot index to load the process into.
 * @return ENONE on success, negative error code on failure.
 */
int process_load_into_slot(const char* filename, process_t** out_process, uint16_t process_slot) {
    int res = 0;
    process_t* process = NULL;

    // Check if the slot is valid
    if (process_get_process_by_slot(process_slot) != NULL) {
        return -EBUSY; // Slot already occupied
    }

    // Check if out_process is valid
    if (out_process == NULL) {
        return -EINVAL;
    }

    // Allocate memory for the new process structure
    process = (process_t*)kheap_zmalloc(sizeof(process_t));
    if (!process) {
        res = -ENOMEM;
        goto exit;
    }

    // Initialize process fields
    process->pid = process_slot; // Assign PID based on slot
    strncpy(process->filename, filename, sizeof(process->filename) - 1);

    // Load the executable file into memory and populate the process structure, i.e. file_ptr and file_size
    res = process_load_binary(filename, process);
    if (res < 0) {
        goto exit;
    }

    // Create the main task for the process
    process->main_task = task_new(process);
    if (!process->main_task) {
        res = -ENOMEM;
        goto exit;
    }

    // Allocate stack for the process
    process->stack = kheap_zmalloc(PROGRAM_VIRTUAL_STACK_SIZE_BYTES);
    if (!process->stack) {
        res = -ENOMEM;
        goto exit;
    }

    // Map main task's physical memory to virtual address space including binary and stack
    res = process_map_memory(process);
    if (res < 0) {
        goto exit;
    }

    // Insert the process into the process table and set output parameter
    process_table[process_slot] = process;
    *out_process = process;

exit:
    if (res < 0) {
        // Cleanup on error
        if (process) {
            // Free associated resources
            if (process->main_task) {
                task_free(process->main_task);
            }
            // TODO: Free all the process data
            // kheap_free(process);
        }
    }
    return res;    
}

process_t* process_get_current() {
    task_t* current_task = task_get_current();
    if (!current_task) {
        return NULL;
    }
    return current_task->process;
}

process_t* process_get_by_pid(uint16_t pid) {
    if (pid >= PROGRAM_MAX_PROCESSES) {
        return NULL;
    }
    return process_table[pid];
}