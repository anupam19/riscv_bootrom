#ifndef UART_H
#define UART_H

#include <stdint.h>

void uart_init(void);
void uart_putc(char c);
void uart_puts(const char *s);
void uart_put_hex8(uint8_t val);
void uart_put_hex32(uint32_t val);
void uart_put_hex64(uint64_t val);

#endif /* UART_H */
