#ifndef UART_H
#define UART_H

#include <stdint.h>

// Initialize UART with standard settings (115200 baud, 8N1)
void uart_init(void);

// Send a single character
//void uart_putc(char c);

// Send a string
void uart_puts(const char *s);

// Get a character (blocking)
char uart_getc(void);

// Check if a character is available
int uart_available(void);

#endif /* UART_H */