#ifndef __IO_H__
#define __IO_H__

#include <stdint.h>

// Function prototypes for Input/Output Ports
/**
 * @brief Reads a byte from the specified I/O port.
 * @param port The I/O port to read from.
 * @return The byte read from the port.
 */
uint8_t inb(uint16_t port);
/**
 * @brief Reads a word (2 bytes) from the specified I/O port.
 * @param port The I/O port to read from.
 * @return The word read from the port.
 */
uint16_t inw(uint16_t port);

/**
 * @brief Writes a byte to the specified I/O port.
 * @param port The I/O port to write to.
 * @param data The byte to write to the port.
 */
void outb(uint16_t port, uint8_t data);
/**
 * @brief Writes a word (2 bytes) to the specified I/O port.
 * @param port The I/O port to write to.
 * @param data The word to write to the port.
 */
void outw(uint16_t port, uint16_t data);


#endif // __IO_H__