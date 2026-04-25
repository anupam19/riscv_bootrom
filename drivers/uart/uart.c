#include "uart.h"

/* UART base address - platform configurable */
#ifndef UART_BASE_ADDR
#define UART_BASE_ADDR 0x10000000
#endif

volatile uint8_t *const uart_base = (volatile uint8_t *const) UART_BASE_ADDR;

void uart_init(void)
{
    /* For actual hardware, set baud rate, data bits, stop bits, parity */
    /* Example: 16550 UART at 115200 baud, PCLK=25MHz */
    /* Divisor = 25MHz / (16 * 115200) ≈ 13.57 → DL=13, fraction=0x1B */
    /* Write to DLL, DLM, LCR, etc. */
    /* This is a stub - actual init depends on specific UART IP */
}

void uart_putc(char c)
{
    volatile uint8_t *reg = uart_base;
    /* Wait for THR empty (LSR[5]) */
    while (!(reg[5] & 0x20)) {
        ;
    }
    reg[0] = c; /* THR */
}

void uart_puts(const char *s)
{
    while (*s) {
        if (*s == '\n') {
            uart_putc('\r');
        }
        uart_putc(*s++);
    }
}
