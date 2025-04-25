
// PMP trap test for TH1520 (T-HEAD C910)
// Load via fastboot as usual; targets SRAM at 0xFFE0000000

#include <stdint.h>

#define UART_BASE 0xFFE7014000ULL
#define UART_THR  (UART_BASE + 0x00)
#define UART_LSR  (UART_BASE + 0x14)
#define UART_LSR_THRE 0x20

static inline void putc(char c) {
    while (!(*(volatile uint32_t *)UART_LSR & UART_LSR_THRE));
    *(volatile uint32_t *)UART_THR = c;
}

static void puts(const char *s) {
    while (*s) putc(*s++);
}

static void puthex(uint64_t val) {
    const char hex[] = "0123456789ABCDEF";
    for (int i = 60; i >= 0; i -= 4)
        putc(hex[(val >> i) & 0xF]);
    putc('\n');
}

__attribute__((section(".pmp_data"), used, aligned(4)))
volatile uint64_t protected_data = 0xCAFEBABE;

void trap_handler(void) {
    puts("TRAP HANDLER\n");
    uint64_t mcause, mepc;
    asm volatile ("csrr %0, mcause" : "=r"(mcause));
    asm volatile ("csrr %0, mepc" : "=r"(mepc));
    puts("mcause: "); puthex(mcause);
    puts("mepc: "); puthex(mepc);
    while (1);
}

void main() {
    puts("Booting\n");

    asm volatile("csrw mtvec, %0" :: "r"(trap_handler));

    uintptr_t base = 0xFFE0000000 >> 2;
    uintptr_t limit = 0xFFE0180000 >> 2;
    uintptr_t cfg = 0x83;

    asm volatile (
        "csrw pmpaddr0, %0\n"
        "csrw pmpaddr1, %1\n"
        "csrw pmpcfg0, %2\n"
        :: "r"(base), "r"(limit), "r"(cfg)
        : "memory"
    );

    asm volatile("csrw satp, zero");
    uint64_t satp;
    asm volatile("csrr %0, satp" : "=r"(satp));
    puts("satp: "); puthex(satp);

    uintptr_t smode_entry = (uintptr_t)&&s_mode;
    asm volatile("csrw mepc, %0\n"
                 "csrr a0, mstatus\n"
                 "li a1, ~(3 << 11)\n"
                 "and a0, a0, a1\n"
                 "li a1, (1 << 11)\n"
                 "or a0, a0, a1\n"
                 "csrw mstatus, a0\n"
                 "mret" :: "r"(smode_entry) : "a0", "a1");

s_mode:
    puts("In S-mode\n");
    protected_data = 0xDEADBEEF;
    puts("S-mode write succeeded (unexpected)\n");
    while (1);
}

__attribute__((section(".text.start")))
void _start() {
    main();
}
