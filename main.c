// Minimal PMP Enforcement Test for TH1520 (T-HEAD C910)
// Runs entirely in M-mode and triggers a store fault using PMP

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

// Memory we want to protect
__attribute__((section(".pmp_data"), used, aligned(4)))
volatile uint64_t protected_data = 0xCAFEBABE;

void user_mode_test() {
    puts("Now in U-mode\r\n");

    // Read protected_data (should succeed)
    uint64_t read_value = protected_data;
    puts("Read value: "); puthex(read_value); puts("\r");

    // Attempt to write protected_data (should trap)
    puts("Attempting to write protected_data from U-mode (should trap)...\r\n");
    protected_data = 0xDEADBEEF;

    puts("Write succeeded (unexpected)\r\n");

    read_value = protected_data;
    puts("Read value: "); puthex(read_value); puts("\r\n");

    while (1);
}

// Enhanced trap handler
void trap_handler(void) {
    puts("TRAP HANDLER\r\n");
    uint64_t mcause, mepc, mtval, mstatus;
    asm volatile ("csrr %0, mcause" : "=r"(mcause));
    asm volatile ("csrr %0, mepc" : "=r"(mepc));
    asm volatile ("csrr %0, mtval" : "=r"(mtval));
    asm volatile ("csrr %0, mstatus" : "=r"(mstatus));

    puts("mcause: "); puthex(mcause); puts("\r\n");
    puts("mepc: "); puthex(mepc); puts("\r\n");
    puts("mtval: "); puthex(mtval); puts("\r\n");
    puts("mstatus: "); puthex(mstatus); puts("\r\n");

    puts("PMP protection triggered successfully!\r\n");

    // Return to M-mode and halt
    while (1);
}

void main() {
    puts("Booting\r\n");

    // Set up trap vector
    asm volatile("csrw mtvec, %0" :: "r"(trap_handler));

    // Report address of protected data
    puts("protected_data: "); puthex((uintptr_t)&protected_data); puts("\r");
//read initial pmp values
uint64_t pmpaddr0, pmpaddr1, pmpcfg0;
asm volatile ("csrr %0, pmpaddr0" : "=r"(pmpaddr0));
asm volatile ("csrr %0, pmpaddr1" : "=r"(pmpaddr1));
asm volatile ("csrr %0, pmpcfg0" : "=r"(pmpcfg0));
puts("pmpaddr0 initially: "); puthex(pmpaddr0); puts("\r");
puts("pmpaddr1 initially: "); puthex(pmpaddr1); puts("\r");
puts("pmpcfg0 initially: "); puthex(pmpcfg0); puts("\r");

// Set up PMP region: R-only for protected_data (enforce in U-mode)
uintptr_t base = ((uintptr_t)&protected_data) >> 2;
uintptr_t limit = ((uintptr_t)&protected_data + sizeof(protected_data)) >> 2;
// Configure PMP:
// - TOR (Top of Range) mode (A=01)
// - Read-only permission (R=1, W=0, X=0)
// - Not locked (L=0)
uintptr_t cfg = 0x09; // TOR, R-only, not locked

// Print calculated values
puts("base: "); puthex(base); puts("\r");
puts("limit: "); puthex(limit); puts("\r");
puts("cfg: "); puthex(cfg); puts("\r");

asm volatile (
    "csrw pmpaddr0, %0\n"
    "csrw pmpaddr1, %1\n"
    "csrw pmpcfg0, %2\n"
    :: "r"(base), "r"(limit), "r"(cfg)
    : "memory"
);

// Read back to verify
asm volatile ("csrr %0, pmpaddr0" : "=r"(pmpaddr0));
asm volatile ("csrr %0, pmpaddr1" : "=r"(pmpaddr1));
asm volatile ("csrr %0, pmpcfg0" : "=r"(pmpcfg0));
puts("pmpaddr0: "); puthex(pmpaddr0); puts("\r");
puts("pmpaddr1: "); puthex(pmpaddr1); puts("\r");
puts("pmpcfg0: "); puthex(pmpcfg0); puts("\r");

// Now switch to U-mode and attempt the write
puts("Switching to U-mode...\r\n");
    // Setup status and return address for U-mode
    uint64_t mstatus_value = 0;
    asm volatile ("csrr %0, mstatus" : "=r"(mstatus_value));

    // Clear MPP bits (bits 12-11) to set to U-mode (00)
    mstatus_value &= ~(3ULL << 11);

    // Set MPIE (bit 7) to enable interrupts when we switch
    mstatus_value |= (1ULL << 7);

    // Write back to mstatus
    asm volatile ("csrw mstatus, %0" :: "r"(mstatus_value));

    // Set up the return address to go to our user_mode_test function
    asm volatile ("csrw mepc, %0" :: "r"(user_mode_test));

    // Jump to U-mode
    asm volatile ("mret");

    // Code should not reach here
    puts("Returned to M-mode unexpectedly\r\n");
    while (1);
}

__attribute__((section(".text.start")))
void _start() {
    main();
}
