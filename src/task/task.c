#include "task.h"
#include "memory/memory.h"
#include "memory/paging/paging.h"
#include "memory/heap/kheap.h"
#include "status.h"
#include "config.h"
#include "kernel.h"

/**
 * A linked list to hold all tasks in the system.
 */
task_t* task_list_head = NULL;
task_t* task_list_tail = NULL;
task_t* current_task = NULL;

void task_list_remove(task_t* task) {
    if (!task) {
        return;
    }

    // Unlink from the list
    if (task->prev) {
        task->prev->next = task->next;
    } else if (task_list_head == task) {
        task_list_head = task->next; // Update head if necessary
    } else {
        // Task not found in list
        return;
    }

    if (task->next) {
        task->next->prev = task->prev;
    } else if (task_list_tail == task) {
        task_list_tail = task->prev; // Update tail if necessary
    } else {
        // Task not found in list
        return;
    }
}

/**
 * @brief Initialize a new task structure.
 * @param task Pointer to the task structure to initialize.
 * @param process Pointer to the associated process.
 * @return ENONE on success, negtive error code on failure.
 
 */
int task_init(task_t* task, process_t* process) {
    if (!task) {
        return EINVAL;
    }

    // Reset task fields
    memset(task, 0, sizeof(task_t));
    task->pid = 0; // Assign a PID as necessary
    task->paging_chunk = paging_4gb_chunk_init(PAGING_FLAG_PRESENT | PAGING_FLAG_USER); // Maping chunk for the task
    if (!task->paging_chunk) {
        return -ENOMEM;
    }
    // Initialize registers as necessary
    task->registers.eip = PROGRAM_VIRTUAL_ADDRESS; // Entry point
    task->registers.ss = USER_DATA_SELECTOR | RPL_USER;
    task->registers.cs = USER_CODE_SELECTOR | RPL_USER;
    task->registers.user_esp = PROGRAM_VIRTUAL_STACK_TOP_ADDRESS; // User stack pointer
    task->process = process;

    return ENONE;
}

/**
 * @brief Create a new task and append it to the tail of the task list.
 * @param process Pointer to the associated process.
 * @return Pointer to the newly created task, or NULL on failure.
 */
task_t* task_new(process_t* process) {
    task_t* new_task = (task_t*)kheap_zmalloc(sizeof(task_t));
    if (!new_task) {
        return NULL;
    }

    if (task_init(new_task, process) != ENONE) {
        kheap_free(new_task);
        return NULL;
    }

    // Add to the task list
    if (!task_list_head) {
        task_list_head = new_task;
        task_list_tail = new_task;
        current_task = new_task;
    } else {
        task_list_tail->next = new_task;
        new_task->prev = task_list_tail;
        task_list_tail = new_task;
    }

    return new_task;
}

int task_free(task_t* task) {
    if (!task) {
        return EINVAL;
    }

    // Unlink from the task list
    task_list_remove(task);

    // Free paging chunk
    if (task->paging_chunk) {
        paging_4gb_chunk_free(task->paging_chunk);
    }

    // Free the task structure
    kheap_free(task);

    return ENONE;
}

/**
 * @brief Get the currently running task.
 * @return Pointer to the current task.
 */
task_t* task_get_current() {
    return current_task;
}

/**
 * @brief Get the next task in the task list for scheduling.
 * @return Pointer to the next task, or NULL if no tasks are available.
 */
task_t* task_get_next() {
    if (!current_task) {
        return task_list_head;
    }
    return current_task->next;
}

/**
 * @brief Switch to the specified next task.
 * @param next_task Pointer to the task to switch to.
 * @return ENONE on success, negative error code on failure.
 */
int task_switch(task_t* next_task) {
    if (!next_task) {
        return EINVAL;
    }

    // Switch paging directory to the next task's paging chunk
    if (next_task->paging_chunk && next_task->paging_chunk->directory_ptr) {
        paging_switch_4gb_chunk(next_task->paging_chunk);
    } else {
        return -EIO; // Paging chunk not set up correctly
    }

    current_task = next_task;
    return ENONE;
}

/**
 * @brief Handle paging for the current task after a context switch.
 * @return ENONE on success, negative error code on failure.
 */
int task_page() {
    if (!current_task || !current_task->paging_chunk) {
        return EINVAL;
    }

    // Retrieve user's data segment selector and switch to current task's paging chunk
    task_restore_user_data_segment();
    task_switch(current_task);
    return ENONE;
}

void task_run_first_ever_task() {
    if (!task_list_head) {
        panic("No tasks available to run.");
    }

    current_task = task_list_head;
    task_switch(current_task);
    task_return_to_user_mode(&current_task->registers);
}