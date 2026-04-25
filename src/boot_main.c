#include "boot.h"
#include "uart.h"

void boot_main(void)
{
#if ENABLE_UART_DEBUG
    uart_init();
    uart_puts("BootROM: Starting...\n");
#endif

    platform_init();

#if ENABLE_UART_DEBUG
    uart_puts("BootROM: Jumping to next stage\n");
#endif

    /* Jump to next stage (BL2 or bootloader)
       Address must be provided by platform or hardcoded */
    void (*next_stage)(void) = (void (*)(void)) 0x80020000;
    next_stage();

    /* In case jump fails, loop forever */
    while (1) {
        __asm__ volatile("wfi");
    }
}
