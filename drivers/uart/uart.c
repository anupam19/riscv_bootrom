void uart_putc(char c)
{
    volatile uint8_t *reg = uart_base;
    /* Wait for THR empty (LSR[5]) */
    while (!(reg[5] & 0x20)) {
        ;
    }
    reg[0] = c; /* THR */

    /* Ensure write completes before continuing (MMIO ordering) */
    __asm__ volatile ("fence w, w" ::: "memory");
}
