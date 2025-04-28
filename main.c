// UART definitions
#define UART_BASE 0xFFE7014000ULL  // UART0 base address
#define UART_THR  (UART_BASE + 0x00)  // Transmitter Holding Register
#define UART_LSR  (UART_BASE + 0x14)  // Line Status Register
#define UART_LSR_THRE 0x20  // THR Empty bit

// PMP definitions
#define PMP_R   0x01
#define PMP_W   0x02
#define PMP_X   0x04
#define PMP_A_NAPOT 0x18
#define PMP_L   0x80

// CSR addresses
#define CSR_MTVEC 0x305

// MMIO read/write
static inline void mmio_write(unsigned long long addr, unsigned int value) {
    *(volatile unsigned int*)addr = value;
}

static inline unsigned int mmio_read(unsigned long long addr) {
    return *(volatile unsigned int*)addr;
}

// CSR write
static inline void csr_write(unsigned int csr, unsigned long long val) {
    __asm__ volatile ("csrw %0, %1" :: "i"(csr), "r"(val));
}

// UART send single character
void uart_putc(char c) {
    while ((mmio_read(UART_LSR) & UART_LSR_THRE) == 0);
    mmio_write(UART_THR, c);
}

// UART send string
void uart_puts(const char *str) {
    while (*str) {
        uart_putc(*str++);
    }
}

// Setup a PMP region
static void pmp_set_region(int pmp_index, unsigned long long addr, unsigned long long size, unsigned char perm)
{
    unsigned long long napot_addr;
    unsigned long long cfg;

    napot_addr = (addr >> 2) | (((size >> 3) - 1));

    switch (pmp_index) {
        case 0: csr_write(0x3B0, napot_addr); break; // pmpaddr0
        case 1: csr_write(0x3B1, napot_addr); break; // pmpaddr1
    }

    cfg = perm | PMP_A_NAPOT;

    switch (pmp_index) {
        case 0: csr_write(0x3A0, cfg); break; // pmpcfg0[7:0]
        case 1: csr_write(0x3A0, cfg << 8); break; // pmpcfg0[15:8]
    }
}

// Setup PMP
void pmp_setup(void)
{
    pmp_set_region(0, 0x00000000, 16 * 1024 * 1024, PMP_R | PMP_W | PMP_X); // 16MB RAM
    pmp_set_region(1, 0xFFE7014000, 0x400, PMP_R | PMP_W); // UART MMIO
}

// Trap handler
void trap_handler(void)
{
    uart_puts("\r\nTRAP: PMP VIOLATION!\r\n");
    while (1) {
        // Loop forever
    }
}

// C entry point
void main(void) {
    uart_puts("Baremetal Hello, World!\r\n");

    uart_puts("Now causing a PMP trap by reading from illegal address...\r\n");

    volatile unsigned int illegal_value = *(volatile unsigned int *)0x10000000; // <-- illegal access

    uart_puts("Survived illegal access! Value = ");
    uart_putc('0' + (illegal_value & 0xF));
    uart_puts("\r\n");

    while (1) {}
}

// Assembly entry point - this will call our C main function
__attribute__((section(".text.start")))
void _start(void) {
    // Set mtvec to point to trap_handler
    csr_write(CSR_MTVEC, (unsigned long long)trap_handler);

    pmp_setup(); // Setup PMP
    main();
}
