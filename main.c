#define UART_BASE 0xFFE7014000ULL
#define UART_THR  (UART_BASE + 0x00)
#define UART_LSR  (UART_BASE + 0x14)
#define UART_LSR_THRE 0x20

#include <stdint.h>

static inline void mmio_write(uint64_t addr, uint32_t value) {
    *(volatile uint32_t*)addr = value;
}

static inline uint32_t mmio_read(uint64_t addr) {
    return *(volatile uint32_t*)addr;
}

void uart_putc(char c) {
    while ((mmio_read(UART_LSR) & UART_LSR_THRE) == 0);
    mmio_write(UART_THR, c);
}

void uart_puts(const char *s) {
    while (*s) uart_putc(*s++);
}

void uart_puthex(uint64_t val) {
    const char hex[] = "0123456789ABCDEF";
    for (int i = 60; i >= 0; i -= 4)
        uart_putc(hex[(val >> i) & 0xF]);
    uart_putc('\n');
}

__attribute__((section(".pmp_data"), used, aligned(4)))
volatile uint64_t protected_data = 0xCAFEBABE;

void trap_handler(void) {
    uart_puts("TRAP HANDLER\n");
    uint64_t mcause, mepc;
    asm volatile ("csrr %0, mcause" : "=r"(mcause));
    asm volatile ("csrr %0, mepc" : "=r"(mepc));
    uart_puts("mcause: "); uart_puthex(mcause);
    uart_puts("mepc: "); uart_puthex(mepc);
    while (1);
}

void set_mtvec(void) {
    uintptr_t handler = (uintptr_t)&trap_handler;
    asm volatile ("csrw mtvec, %0" :: "r"(handler));
}

void main(void) {
    uart_puts("Booting\n");
    set_mtvec();

    uart_puts("protected_data: ");
    uart_puthex((uintptr_t)&protected_data);

    // Read and print mseccfg
    uint64_t mseccfg;
    asm volatile ("csrr %0, 0x747" : "=r"(mseccfg));
    uart_puts("mseccfg: ");
    uart_puthex(mseccfg);

    if (mseccfg & 1) {
        uart_puts("PMP is LOCKED (cannot modify PMP entries)\n");
    } else {
        uart_puts("PMP is UNLOCKED (safe to configure PMP)\n");

        uintptr_t base = 0xFFE0000000 >> 2;
        uintptr_t limit = 0xFFE0180000 >> 2;
        uintptr_t cfg = 0x8F; // TOR | R | W | X

        asm volatile (
            "csrw pmpaddr0, %0\n"
            "csrw pmpaddr1, %1\n"
            "csrw pmpcfg0, %2\n"
            :: "r"(base), "r"(limit), "r"(cfg)
            : "memory"
        );
    }

    // Read back pmpcfg and addr
    uint64_t pmpcfg0, pmpaddr0, pmpaddr1;
    asm volatile ("csrr %0, pmpcfg0" : "=r"(pmpcfg0));
    asm volatile ("csrr %0, pmpaddr0" : "=r"(pmpaddr0));
    asm volatile ("csrr %0, pmpaddr1" : "=r"(pmpaddr1));

    uart_puts("pmpcfg0: "); uart_puthex(pmpcfg0);
    uart_puts("pmpaddr0: "); uart_puthex(pmpaddr0);
    uart_puts("pmpaddr1: "); uart_puthex(pmpaddr1);

    // Disable MMU
    asm volatile("csrw satp, zero");
    uint64_t satp_val;
    asm volatile("csrr %0, satp" : "=r"(satp_val));
    uart_puts("satp: "); uart_puthex(satp_val);

    uart_puts("Trying write to protected_data...\n");
    protected_data = 0xBADBADBADBADBAD;
    uart_puts("Write succeeded (unexpected if PMP was active)\n");

    while (1);
}
