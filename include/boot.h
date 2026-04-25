#ifndef BOOT_H
#define BOOT_H

#include <stdint.h>

void boot_main(void) __attribute__((noreturn));

extern uintptr_t _start;
extern uintptr_t _stack_top;

extern uintptr_t __bss_start;
extern uintptr_t __bss_end;

typedef uintptr_t addr_t;

#define BOOT_OK 0
#define BOOT_ERR -1

#ifndef ENABLE_UART_DEBUG
#define ENABLE_UART_DEBUG 1
#endif

#endif /* BOOT_H */
