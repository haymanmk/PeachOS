/**
 * @file status.h
 * @brief Status codes and related definitions.
 */

#ifndef STATUS_H
#define STATUS_H

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
    ENOTFOUND = 5 // Not found
} error_code_t;

typedef int status_t;
typedef int error_t;

#endif // STATUS_H