#include "uart.h"

/* UART base address - platform configurable */
#ifndef UART_BASE_ADDR
#define UART_BASE_ADDR 0x10000000
#endif

volatile uint8_t *const uart_base = (volatile uint8_t *)UART_BASE_ADDR;

void uart_init(void)
{
    /* Minimal init — assume 16550 default baud */
}

void uart_putc(char c)
{
    volatile uint8_t *reg = uart_base;
    while (!(reg[5] & 0x20)) {
        /* Insert a pause hint to reduce power consumption and pipeline pressure during spin */
         __asm__ volatile ("addi x0, x0, 1");
    }
    reg[0] = c; /* THR */

    /* Ensure write completes before continuing (MMIO ordering) */
    __asm__ volatile ("fence w, w" ::: "memory");
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

/* Hex digit output */
void uart_put_hex8(uint8_t val)
{
    static const char digits[] = "0123456789abcdef";
    uart_putc(digits[(val >> 4) & 0xF]);
    uart_putc(digits[val & 0xF]);
}

void uart_put_hex32(uint32_t val)
{
    for (int i = 3; i >= 0; i--) {
        uart_put_hex8((val >> (i * 8)) & 0xFF);
    }
}

void uart_put_hex64(uint64_t val)
{
    for (int i = 7; i >= 0; i--) {
        uart_put_hex8((val >> (i * 8)) & 0xFF);
    }
}
