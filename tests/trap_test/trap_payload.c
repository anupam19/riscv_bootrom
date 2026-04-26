/* Trap test payload: deliberately triggers an ECALL from M-mode
   to exercise the BootROM trap handler.

   Execution flow:
   1. Print "TRAP_TEST: triggering ECALL..."
   2. Execute ECALL instruction (encoded 0x00000073)
   3. (Control never returns — trap handler takes over and loops on WFI)

   The test runner verifies the trap handler output contains:
   "ECALL from M-mode"
*/

#ifndef UART_BASE_ADDR
#define UART_BASE_ADDR 0x10000000
#endif

#ifndef TEST_FINISHER_ADDR
#define TEST_FINISHER_ADDR 0x100000
#endif

volatile unsigned char *const uart = (volatile unsigned char *)UART_BASE_ADDR;

static inline void uart_wait_tx_ready(void)
{
    while (!(uart[5] & 0x20)) {
        __asm__ volatile("addi x0, x0, 1");
    }
}

static void uart_putc(char c)
{
    uart_wait_tx_ready();
    if (c == '\n') {
        uart[0] = '\r';
        __asm__ volatile("fence w, w" ::: "memory");
        uart_wait_tx_ready();
        uart[0] = '\n';
    } else {
        uart[0] = c;
    }
    __asm__ volatile("fence w, w" ::: "memory");
}

static void uart_puts(const char *s)
{
    while (*s) {
        uart_putc(*s++);
    }
}

void _start(void)
{
    uart_puts("TRAP_TEST: triggering ECALL...\r\n");

    /* Small delay to ensure message is transmitted before trap */
    for (volatile int i = 0; i < 10000; i++) {
    }

    /* ECALL from M-mode — causes trap, BootROM handler prints
       "ECALL from M-mode" and then loops on wfi */
    __asm__ volatile(".word 0x00000073" ::: "memory");

    /* Should never reach here */
    while (1) {
        __asm__ volatile("wfi");
    }
}
