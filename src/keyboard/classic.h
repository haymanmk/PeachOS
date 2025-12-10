#ifndef __KEYBOARD_CLASSIC_H__
#define __KEYBOARD_CLASSIC_H__

#include "keyboard.h"

// Forward declaration
typedef struct keyboard_driver keyboard_driver_t;

#define CLASSIC_I8042_DATA_PORT 0x60
#define CLASSIC_I8042_STATUS_PORT 0x64
#define CLASSIC_I8042_COMMAND_PORT 0x64
#define CLASSIC_I8042_ENABLE_FIRST_PORT 0xAE

// Classic keyboard driver initialization function
keyboard_driver_t* classic_keyboard_driver_init();

#endif // __KEYBOARD_CLASSIC_H__