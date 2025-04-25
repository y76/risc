
#include <stdint.h>

#define UART_BASE 0xFFE7014000ULL
#define UART_THR  (UART_BASE + 0x00)
#define UART_LSR  (UART_BASE + 0x14)
#define UART_LSR_THRE 0x20

#define PMP_BASE_ADDR 0xFFDC020000ULL

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

volatile int mem_access_failed = 0;

void trap_handler(void);
void mem_access_trap_handler(void);

static inline void sync_is(void) {
    asm volatile ("fence.i");
}

void mem_access_trap_handler(void) {
    mem_access_failed = 1;

    uint64_t mcause, mepc;
    asm volatile ("csrr %0, mcause" : "=r"(mcause));
    asm volatile ("csrr %0, mepc" : "=r"(mepc));

    puts("Memory access trapped: mcause="); puthex(mcause);
    puts(" mepc="); puthex(mepc); puts("\r\n");

    asm volatile ("csrw mepc, %0" :: "r"(mepc + 4));

    asm volatile ("mret");
}

void configure_pmp(void) {
    puts("Configuring PMP using memory-mapped interface...\r\n");

    uint64_t old_mtvec;
    asm volatile ("csrr %0, mtvec" : "=r"(old_mtvec));
    asm volatile ("csrw mtvec, %0" :: "r"(mem_access_trap_handler));

    mem_access_failed = 0;

    if (!mem_access_failed) {
        uint32_t pmp_cfg = *(volatile uint32_t*)(PMP_BASE_ADDR + 0x000);
        uint32_t pmpaddr0 = *(volatile uint32_t*)(PMP_BASE_ADDR + 0x100);
        uint32_t pmpaddr1 = *(volatile uint32_t*)(PMP_BASE_ADDR + 0x104);
        uint32_t pmpaddr2 = *(volatile uint32_t*)(PMP_BASE_ADDR + 0x108);
        uint32_t pmpaddr3 = *(volatile uint32_t*)(PMP_BASE_ADDR + 0x10C);

        puts("Current PMP config:\r\n");
        puts("pmpcfg0: "); puthex(pmp_cfg); puts("\r");
        puts("pmpaddr0: "); puthex(pmpaddr0); puts("\r");
        puts("pmpaddr1: "); puthex(pmpaddr1); puts("\r");
        puts("pmpaddr2: "); puthex(pmpaddr2); puts("\r");
        puts("pmpaddr3: "); puthex(pmpaddr3); puts("\r");
    }

    uintptr_t base = ((uintptr_t)&protected_data) >> 12;
    uintptr_t limit = ((uintptr_t)&protected_data + sizeof(protected_data)) >> 12;

    puts("Protected memory region:\r\n");
    puts("Base address: "); puthex((uintptr_t)&protected_data); puts("\r");
    puts("Base >> 12: "); puthex(base); puts("\r");
    puts("Limit >> 12: "); puthex(limit); puts("\r");

    if (!mem_access_failed) {
        *(volatile uint32_t*)(PMP_BASE_ADDR + 0x104) = 0;
        *(volatile uint32_t*)(PMP_BASE_ADDR + 0x100) = 0;
        *(volatile uint32_t*)(PMP_BASE_ADDR + 0x10C) = 0;
        *(volatile uint32_t*)(PMP_BASE_ADDR + 0x108) = 0;
        *(volatile uint32_t*)(PMP_BASE_ADDR + 0x000) = 0;


        *(volatile uint32_t*)(PMP_BASE_ADDR + 0x100) = base;    // Base address
        *(volatile uint32_t*)(PMP_BASE_ADDR + 0x104) = limit;   // Limit address


        *(volatile uint32_t*)(PMP_BASE_ADDR + 0x000) = 0x9;

        sync_is();

        uint32_t pmp_cfg_after = *(volatile uint32_t*)(PMP_BASE_ADDR + 0x000);
        uint32_t pmpaddr0_after = *(volatile uint32_t*)(PMP_BASE_ADDR + 0x100);
        uint32_t pmpaddr1_after = *(volatile uint32_t*)(PMP_BASE_ADDR + 0x104);

        puts("Updated PMP config:\r\n");
        puts("pmpcfg0: "); puthex(pmp_cfg_after); puts("\r");
        puts("pmpaddr0: "); puthex(pmpaddr0_after); puts("\r");
        puts("pmpaddr1: "); puthex(pmpaddr1_after); puts("\r");
    }

    asm volatile ("csrw mtvec, %0" :: "r"(old_mtvec));

    if (mem_access_failed) {
        puts("Some PMP accesses failed with exceptions\r\n");
    } else {
        puts("PMP configuration successful\r\n");
    }
}

void user_mode_test() {
    puts("Now in U-mode\r\n");

    uint64_t read_value = protected_data;
    puts("Read value: "); puthex(read_value); puts("\r");

    puts("Attempting to write protected_data from U-mode (should trap)...\r\n");
    protected_data = 0xDEADBEEF;

    puts("Write succeeded (unexpected)\r\n");

    read_value = protected_data;
    puts("Read value: "); puthex(read_value); puts("\r\n");

    while (1);
}

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

    while (1);
}

void main() {
    puts("Booting\r\n");

    asm volatile("csrw mtvec, %0" :: "r"(trap_handler));

    puts("protected_data: "); puthex((uintptr_t)&protected_data); puts("\r");

    configure_pmp();

    puts("Switching to U-mode...\r\n");

    uint64_t mstatus_value = 0;
    asm volatile ("csrr %0, mstatus" : "=r"(mstatus_value));

    mstatus_value &= ~(3ULL << 11);

    mstatus_value |= (1ULL << 7);

    asm volatile ("csrw mstatus, %0" :: "r"(mstatus_value));

    asm volatile ("csrw mepc, %0" :: "r"(user_mode_test));

    asm volatile ("mret");

    puts("Returned to M-mode unexpectedly\r\n");
    while (1);
}

__attribute__((section(".text.start")))
void _start() {
    main();
}
