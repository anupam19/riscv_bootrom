/* Minimal bare-metal RISC-V payload for BootROM integration testing.
   No standard library. Uses direct MMIO to UART and SiFive Test Finisher.
   Entry point: _start at 0x80020000.
   - Prints "PAYLOAD: OK\r\n" over UART
   - Writes 0x5555 to SiFive Test Finisher to signal QEMU success exit
   - Halts
*/

#ifndef UART_BASE_ADDR
#define UART_BASE_ADDR 0x10000000
#endif

#ifndef TEST_FINISHER_ADDR
#define TEST_FINISHER_ADDR 0x100000
#endif

/* UART MMIO base as a macro to avoid global pointer variables */
#define UART      ((volatile unsigned char *)UART_BASE_ADDR)
#define TEST_FINISHER ((volatile unsigned int *)TEST_FINISHER_ADDR)

/* UART helper: wait for THRE (bit 5 of LSR at offset 5) */
static inline void uart_wait_tx_ready(void)
{
    while (!(UART[5] & 0x20)) {
        __asm__ volatile("addi x0, x0, 1");
    }
}

/* UART putc with CR/LF translation (matches BootROM convention) */
static void uart_putc(char c)
{
    uart_wait_tx_ready();
    if (c == '\n') {
        UART[0] = '\r';
        __asm__ volatile("fence w, w" ::: "memory");
        uart_wait_tx_ready();
        UART[0] = '\n';
    } else {
        UART[0] = c;
    }
    __asm__ volatile("fence w, w" ::: "memory");
}

/* UART puts */
static void uart_puts(const char *s)
{
    while (*s) {
        uart_putc(*s++);
    }
}

/* _start entry point — called via jalr from BootROM */
void _start(void)
{
    uart_puts("PAYLOAD: OK\r\n");

    /* Write 32-bit value to test finisher to exit QEMU */
    TEST_FINISHER[0] = 0x5555;

    /* Should not reach here — test_finisher write causes QEMU exit */
    while (1) {
        __asm__ volatile("wfi");
    }
}
