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
#define TEST_FINISHER ((volatile unsigned short *)TEST_FINISHER_ADDR)

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
    /* Print message without a string literal to avoid memory loads. */
    uart_putc('P');
    uart_putc('A');
    uart_putc('Y');
    uart_putc('L');
    uart_putc('O');
    uart_putc('A');
    uart_putc('D');
    uart_putc(':');
    uart_putc(' ');
    uart_putc('O');
    uart_putc('K');
    uart_putc('\r');
    uart_putc('\n');

    /* Small delay to allow UART to transmit all characters before we
       trigger QEMU exit via the test finisher. */
    for (volatile int i = 0; i < 100000; i++) { }

    /* Ensure all UART writes complete before triggering the test finisher. */
    __asm__ volatile ("fence w, w" ::: "memory");

    /* Write to SiFive Test Finisher to exit QEMU.
       Use 16-bit halfword store; device accepts this and triggers exit. */
    *TEST_FINISHER = (unsigned short)0x5555;

    /* Should not reach here — test_finisher write causes QEMU exit */
    while (1) {
        __asm__ volatile("wfi");
    }
}
