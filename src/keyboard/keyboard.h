#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include "config.h"

#define KEYBOARD_IDT_INTERRUPT_NUMBER (__PIC1_VECTOR_OFFSET + 1) // PIC IRQ1

// Function pointer type for keyboard driver initialization
typedef int (*keyboard_driver_init_func_t)();

typedef struct keyboard_driver {
    keyboard_driver_init_func_t init;
    char name[32];
    struct keyboard_driver* next;
} keyboard_driver_t;

void keyboard_init(); // intialize keyboard system including drivers
int keyboard_register_driver(keyboard_driver_t* driver);
void keyboard_backspace();
void keyboard_push(char c);
char keyboard_pop();

#endif // __KEYBOARD_H__