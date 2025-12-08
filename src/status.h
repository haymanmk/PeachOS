/**
 * @file status.h
 * @brief Status codes and related definitions.
 */

#ifndef STATUS_H
#define STATUS_H

// Cast an error code to a void pointer for functions that return pointers
#define ERROR_VOID(code) (void*)((intptr_t)(code))
#define IS_ERROR(ptr) ((intptr_t)(ptr) < 0)
#define ERROR_CODE(ptr) ((error_t)(intptr_t)(ptr))

// Status codes
typedef enum {
    STATUS_OK = 0,
    STATUS_ERROR = 1,
    STATUS_BUSY = 2
} status_code_t;

// Error codes
typedef enum {
    ENONE = 0, // No error
    EINVAL = 1, // Invalid argument
    EIO = 2, // Input/output error
    ENOMEM = 3, // Out of memory
    EBUSY = 4, // Resource busy
    ENOTFOUND = 5, // Not found
    ENOTDIR = 6, // Not a directory
    ENODATA = 7, // No data available
    EBADF = 8, // Bad file descriptor
    EFAULT = 9 // Operation failed
} error_code_t;

typedef int status_t;
typedef int error_t;

#endif // STATUS_H