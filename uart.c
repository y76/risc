#include "uart.h"
#include <stdint.h>

// Correct UART base address from the DTS file
#define UART_BASE       0xffe7014000

// Register shift value from DTS
#define REG_SHIFT       2

// Calculate register offsets with shift value
#define UART_REG(offset) (offset << REG_SHIFT)

// UART registers
#define UART_RBR        UART_REG(0x00)  // Receive buffer register
#define UART_THR        UART_REG(0x00)  // Transmit holding register
#define UART_IER        UART_REG(0x01)  // Interrupt enable register
#define UART_FCR        UART_REG(0x02)  // FIFO control register
#define UART_LCR        UART_REG(0x03)  // Line control register
#define UART_MCR        UART_REG(0x04)  // Modem control register
#define UART_LSR        UART_REG(0x05)  // Line status register
#define UART_MSR        UART_REG(0x06)  // Modem status register

// Line status register bits
#define UART_LSR_DR     0x01  // Data ready
#define UART_LSR_THRE   0x20  // THR empty

void uart_init(void) {
    // Don't initialize - U-Boot has already done it
    // We'll just use the UART as configured
}
/*
void uart_putc(char c) {
    volatile uint32_t* lsr = (volatile uint32_t*)(UART_BASE + UART_LSR);
    volatile uint32_t* thr = (volatile uint32_t*)(UART_BASE + UART_THR);
    
    // Wait until transmitter holding register is empty
    while (!(*lsr & UART_LSR_THRE));
    
    // Send character
    *thr = c;
    
    // If it's a newline, also send carriage return
    if (c == '\n') {
        uart_putc('\r');
    }
}
*/
void uart_puts(const char *s) {
    while (*s) {
        uart_putc(*s++);
    }
}

char uart_getc(void) {
    volatile uint32_t* lsr = (volatile uint32_t*)(UART_BASE + UART_LSR);
    volatile uint32_t* rbr = (volatile uint32_t*)(UART_BASE + UART_RBR);
    
    // Wait until data is available
    while (!(*lsr & UART_LSR_DR));
    
    // Return received character
    return *rbr;
}

int uart_available(void) {
    volatile uint32_t* lsr = (volatile uint32_t*)(UART_BASE + UART_LSR);
    return (*lsr & UART_LSR_DR);
}