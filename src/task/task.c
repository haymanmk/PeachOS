#include "task.h"
#include "memory/memory.h"
#include "memory/paging/paging.h"
#include "memory/heap/kheap.h"
#include "status.h"
#include "config.h"
#include "kernel.h"
#include "utils/string.h"

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
 * @brief Save the state of the given task from the interrupt stack frame.
 * @param task Pointer to the task whose state is to be saved.
 * @param frame Pointer to the interrupt stack frame containing the CPU state.
 */
void task_save_state(task_t* task, idt_interrupt_stack_frame_t* frame) {
    if (!task || !frame) {
        return;
    }

    // Save general-purpose registers
    task->registers.edi = frame->edi;
    task->registers.esi = frame->esi;
    task->registers.ebp = frame->ebp;
    task->registers.ebx = frame->ebx;
    task->registers.edx = frame->edx;
    task->registers.ecx = frame->ecx;
    task->registers.eax = frame->eax;

    // Save instruction pointer, code segment, flags, user stack pointer, and stack segment
    task->registers.eip = frame->eip;
    task->registers.cs = frame->cs;
    task->registers.eflags = frame->eflags;
    task->registers.user_esp = frame->user_esp;
    task->registers.ss = frame->ss;
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
    task->paging_chunk = paging_4gb_chunk_init(PAGING_FLAG_PRESENT | PAGING_FLAG_USER | PAGING_FLAG_WRITABLE); // Maping chunk for the task
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
int task_page_current() {
    if (!current_task || !current_task->paging_chunk) {
        return EINVAL;
    }

    // Retrieve user's data segment selector and switch to current task's paging chunk
    task_restore_user_data_segment();
    task_switch(current_task);
    return ENONE;
}

/**
 * @brief Handle paging for the specified task.
 * @param task Pointer to the task whose paging is to be handled.
 * @return ENONE on success, negative error code on failure.
 */
int task_page_task(task_t* task) {
    if (!task || !task->paging_chunk) {
        return EINVAL;
    }

    // Retrieve user's data segment selector and switch to specified task's paging chunk
    task_restore_user_data_segment();
    task_switch(task);
    return ENONE;
}

/**
 * @brief Run the first ever task in the task list.
 * This function switches to the first task and returns to user mode.
 */
void task_run_first_ever_task() {
    if (!task_list_head) {
        panic("No tasks available to run.");
    }

    current_task = task_list_head;
    task_switch(current_task);
    task_return_to_user_mode(&current_task->registers);
}

/**
 * @brief Save the current task's state from the interrupt stack frame.
 * @param frame Pointer to the interrupt stack frame containing the CPU state.
 */
void task_save_current_state(idt_interrupt_stack_frame_t* frame) {
    if (!current_task || !frame) {
        return;
    }

    // Save the current task's state such as registers
    task_save_state(current_task, frame);
}

/**
 * @brief Copy a string from the virtual address space of a task to a physical address in the kernel.
 * @param task Pointer to the task from which to copy the string.
 * @param src_virt_addr Source virtual address in the task's address space.
 * @param dest_phys_addr Destination physical address in the kernel space.
 * @param max_length Maximum length of the string to copy. (range: 1 to PAGE_SIZE)
 * @return ENONE on success, negative error code on failure.
 * @note This function may be executed in kernel mode. In order to copy data from user mode tasks,
 *       the paging must be temporarily switched to the task's paging chunk, and then switched back to kernel paging.
 *       A temporary buffer in kernel space is used as an intermediary to facilitate the copy operation.
 *       After the operation, the original page mapping is restored to maintain memory integrity.
 *       For instance, if the temporary buffer overlaps with the task's memory, the original mapping must be restored.
 */
int task_copy_string_from_task(task_t* task, const char* src_virt_addr, char* dest_phys_addr, size_t max_length) {
    if (!task || !src_virt_addr || !dest_phys_addr || max_length == 0) {
        return -EINVAL;
    }
    if (max_length > PAGE_SIZE) {
        return -EINVAL;
    }

    int res = 0;
    // Allocate a temporary buffer in kernel space as a shared area between the task and the kernel
    char* temp_buffer = (char*)kheap_zmalloc(max_length);
    if (!temp_buffer) {
        return -ENOMEM;
    }
    // Save the original page table entry to restore later, because the temp buffer (physical address) may overlap with task's memory
    uint32_t original_page_entry = paging_get_page_entry(task->paging_chunk, (uint32_t)temp_buffer);
    if (original_page_entry == 0) {
        res = -ENOTFOUND; // Page not mapped
        goto exit;
    }
    // Map the temporary buffer (physical address) into the task's paging chunk at the same address of temp_buffer (virtual address)
    res = paging_map_virtual_address(task->paging_chunk, (uint32_t)temp_buffer, (uint32_t)temp_buffer | PAGING_FLAG_PRESENT | PAGING_FLAG_USER | PAGING_FLAG_WRITABLE);
    if (res != ENONE) {
        goto exit;
    }
    // Kernel landscape above
    ////////////////////////////////
    // User landscape below
    // Switch to the task's paging chunk to copy data from its virtual address space
    paging_switch_4gb_chunk(task->paging_chunk);
    // Copy the string from the task's virtual address to the temporary buffer
    strncpy(temp_buffer, src_virt_addr, max_length);
    ////////////////////////////////
    // Kernel landscape below
    // Restore to kernel paging
    kernel_page();
    // Copy the string from the temporary buffer to the destination physical address
    strncpy(dest_phys_addr, temp_buffer, max_length);
    // Restore the original page entry
    res = paging_map_virtual_address(task->paging_chunk, (uint32_t)temp_buffer, original_page_entry);
    if (res != ENONE) {
        goto exit;
    }

exit:
    kheap_free(temp_buffer);
    return res;
}

void* task_get_stack_item(task_t* task, uint32_t index) {
    if (!task) {
        return NULL;
    }

    //////////////////////
    // Kernel landscape //
    //////////////////////
    // This function is supposed to be called in kernel mode.
    // Calculate the address of the stack item based on the index
    // Retrieve the base of the stack from the task's saved user ESP
    uint32_t* stack_base = (uint32_t*)(task->registers.user_esp);
    
    ///////////////////////
    // User paging below //
    ///////////////////////
    // Switch to the task's page to access its stack
    task_page_task(task);

    // Read the stack item
    void* result = (void*)(stack_base[index]);

    /////////////////////////
    // Kernel paging below //
    /////////////////////////
    // Restore to kernel paging
    kernel_page();

    return result;
}