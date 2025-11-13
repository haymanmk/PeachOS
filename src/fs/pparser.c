#include "pparser.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "utils/string.h"
#include <stddef.h>
#include <stdbool.h>

/**
 * @file pparser.c
 * @brief Path parser implementation.
 */

 /**
  * @brief Check if the given path is valid by examining its first 3 characters.
  *        A valid path starts with the drive number followed by a colon and a slash (e.g., "0:/").
  * @param path The path string to validate.
  * @return true if the path is valid, false otherwise.
  */
bool path_is_path_valid(const char* path) {
    uint32_t len = strnlen(path, 3); // We only need to check the first 3 characters, e.g., "0:/"
    if (len < 3) {
        return false; // Path is too short to be valid
    }
    if (!is_digit(path[0]) || path[1] != ':' || path[2] != '/') {
        return false; // Invalid path format
    }
    return true;
}

/**
 * @brief Extract the drive number from the path and advance the path pointer.
 * @param path Pointer to the path string pointer. The pointer will be advanced past the drive specifier.
 * @return The drive number as an integer. Returns 0xFF if the path is invalid.
 */
uint8_t path_get_drive_no(const char** path) {
    // check if the path is valid
    if (!path_is_path_valid(*path)) {
        return 0xFF; // Invalid path, return an invalid drive number
    }
    uint8_t drive_no = char_to_digit((*path)[0]);
    // add 3 to the path to skip "X:/"
    *path += 3;
    return drive_no;
}

/**
 * @brief Create a new path_root_t structure with the specified drive number.
 *        Note: The created structure should be freed by the caller using kheap_free.
 * @param drive_no The drive number to set in the path root.
 * @return Pointer to the newly created path_root_t structure, or NULL on failure.
 */
path_root_t* path_create_root(uint8_t drive_no) {
    path_root_t* root = (path_root_t*)kheap_zmalloc(sizeof(path_root_t));
    if (!root) {
        return NULL; // Memory allocation failed
    }
    root->drive_no = drive_no;
    root->root = NULL;
    return root;
}

/**
 * @brief Extract the next part of the path into the provided buffer.
 * @param path Pointer to the path string pointer. The pointer will be advanced past the extracted part.
 * @param buffer Buffer to store the extracted part.
 * @param buffer_size Size of the buffer.
 * @return The length of the extracted part, or 0 if there are no more parts, or negative on error.
 */
int path_get_path_part(const char** path, char* buffer, size_t buffer_size) {
    // check edge cases
    if (!path || !*path || !buffer || buffer_size == 0) {
        return -EINVAL; // invalid arguments
    }
    if (**path == '/') {
        return -EINVAL; // no part between slashes
    }

    size_t i = 0;
    // Skip leading slashes. Reserve the last character of the buffer for null-terminator.
    while (**path != '/' && **path != '\0' && i < buffer_size - 1) {
        buffer[i++] = **path;
        (*path)++;
    }
    // skip the slash if present
    if (**path == '/') {
        (*path)++;
    }

    return (int)i; // return the length of the part
}

path_part_t* path_parse_path_parts(const char** path) {
    path_part_t* head = NULL;
    path_part_t* tail = NULL;

    char part_buffer[PATH_MAX_PART_NAME_LENGTH];
    int part_length;

    while (true) {
        // get the next part
        part_length = path_get_path_part(path, part_buffer, sizeof(part_buffer));
        if (part_length < 0) {
            // error occurred.
            // TODO: a well defined error handling mechanism should be implemented.
            break;
        }
        if (part_length == 0) {
            // no more parts
            break;
        }
        // null-terminate the part name
        part_buffer[part_length] = '\0';

        // create a new path_part_t
        path_part_t* new_part = (path_part_t*)kheap_zmalloc(sizeof(path_part_t));
        if (!new_part) {
            // memory allocation failed
            goto cleanup;
        }

        new_part->name = (const char*)kheap_zmalloc(part_length + 1);
        if (!new_part->name) {
            kheap_free(new_part);
            goto cleanup;
        }
        strcpy((char*)new_part->name, part_buffer);
        new_part->next = NULL;

        // add the new part to the end of the list
        if (!head) {
            head = new_part;
            tail = new_part;
        } else {
            tail->next = new_part;
            tail = new_part;
        }
    }

    return head;

cleanup:
    // free allocated parts in case of error
    path_part_t* current = head;
    while (current) {
        path_part_t* next = current->next;
        kheap_free(current);
        current = next;
    }
    return NULL;
}

path_root_t* path_parse(const char* path) {
    if (!path) {
        return NULL;
    }

    path_root_t* root = path_create_root(0);
    if (!root) {
        return NULL;
    }

    root->root = path_parse_path_parts(&path);
    if (!root->root) {
        kheap_free(root);
        return NULL;
    }

    return root;
}

/**
 * @brief Free the memory allocated for a parsed path.
 * @param parsed_path Pointer to the parsed path structure to free.
 */
void path_free(path_root_t* parsed_path) {
    path_part_t* current = parsed_path->root;
    while (current) {
        path_part_t* next = current->next;
        kheap_free(current);
        current = next;
    }
    kheap_free(parsed_path);
}