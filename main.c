#include <stdint.h>

// Simple delay function
void delay(uint32_t count) {
    for (volatile uint32_t i = 0; i < count; i++) {
        // Just wasting cycles
    }
}

// UART output function - using the address we confirmed works
void uart_putc(char c) {
    volatile uint32_t* uart_thr = (volatile uint32_t*)0xffe7014000;
    *uart_thr = c;
}

// Main function
int main(void) {
    // Initial delay to ensure everything is stable
    delay(50000000);
    
    // Try to send some characters
    uart_putc('H');
    delay(1000000);
    uart_putc('e');
    delay(1000000);
    uart_putc('l');
    delay(1000000);
    uart_putc('l');
    delay(1000000);
    uart_putc('o');
    delay(1000000);
    uart_putc('\r');
    delay(1000000);
    uart_putc('\n');
    delay(1000000);
    
    // Endless loop
    while (1) {
        uart_putc('.');
        delay(20000000);
    }
    
    return 0;
}