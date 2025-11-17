#ifndef __PPARSER_H__
#define __PPARSER_H__

#include <stdint.h>
#include "config.h"
#include "status.h"

/* Type Definitions */
typedef struct path_part {
    const char* name; // Name of the path part
    struct path_part* next; // Pointer to the next part
} path_part_t;

typedef struct {
    uint8_t drive_no; // Drive number (e.g., 0 for A:, 1 for B:, etc.)
    path_part_t* first; // Pointer to the first directory/file part
} path_root_t;

/* Exported Functions */
path_root_t* path_parse(const char* path);
void path_free(path_root_t* parsed_path);

#endif // __PPARSER_H__