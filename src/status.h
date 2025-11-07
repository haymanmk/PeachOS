/**
 * @file status.h
 * @brief Status codes and related definitions.
 */

#ifndef STATUS_H
#define STATUS_H

// Status codes
#define STATUS_OK 0
#define STATUS_ERROR 1
#define STATUS_BUSY 2

// Error codes
#define ENONE 0
#define EINVALARG 1
#define EIO 2
#define ENOMEM 3
#define EBUSY 4
#define ENOTFOUND 5

typedef int status_t;
typedef int error_t;

#endif // STATUS_H