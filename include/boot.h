#ifndef BOOT_H
#define BOOT_H

#include <stdint.h>

/* BootROM entry point signature (called from assembly) */
void boot_main(void) __attribute__((noreturn));

/* Linker-defined symbols */
extern uintptr_t _start;
extern uintptr_t _stack_top;

/* Memory section boundaries */
extern uintptr_t __bss_start;
extern uintptr_t __bss_end;

/* Simple types */
typedef uintptr_t addr_t;

/* Status codes */
#define BOOT_OK      0
#define BOOT_ERR     -1

/* Debug UART enable */
#ifndef ENABLE_UART_DEBUG
#define ENABLE_UART_DEBUG 1
#endif

#endif /* BOOT_H */
