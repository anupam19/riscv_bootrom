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

    /* Jump to next stage using full 32-bit address construction
       (LUI + ADDI + JALR). This demonstrates PIC-style jump
       without using a direct function call. */

    uintptr_t target = 0x80020000UL;

    __asm__ volatile (
        "lui x10, %0\n\t"
        "addi x10, x10, %1\n\t"
        "jalr x0, x10, 0"
        :
        : "i" (target >> 12), "i" (target & 0xFFF)
        : "x10", "memory");

    while (1) {
        __asm__ volatile("wfi");
    }
}

/* Trap handler — prints diagnostic and halts */
void trap_handler(void)
{
    uintptr_t mcause = csr_read(CSR_MCAUSE);
    uintptr_t mepc = csr_read(CSR_MEPC);
    uintptr_t mtval = csr_read(CSR_MTVAL);
    const char *cause_str;

    if (mcause & (1UL << 31)) {
        switch (mcause & 0xFF) {
        case 0x01:
            cause_str = "Machine Timer";
            break;
        case 0x03:
            cause_str = "Machine External";
            break;
        case 0x05:
            cause_str = "Machine Software";
            break;
        default:
            cause_str = "Unknown interrupt";
            break;
        }
    } else {
        switch (mcause & 0xFF) {
        case 0x00:
            cause_str = "Instruction misaligned";
            break;
        case 0x01:
            cause_str = "Instruction access fault";
            break;
        case 0x02:
            cause_str = "Illegal instruction";
            break;
        case 0x03:
            cause_str = "Breakpoint (EBREAK)";
            break;
        case 0x04:
            cause_str = "Load misaligned";
            break;
        case 0x05:
            cause_str = "Load access fault";
            break;
        case 0x06:
            cause_str = "Store misaligned";
            break;
        case 0x07:
            cause_str = "Store access fault";
            break;
        case 0x08:
            cause_str = "ECALL from U-mode";
            break;
        case 0x09:
            cause_str = "ECALL from S-mode";
            break;
        case 0x0A:
            cause_str = "ECALL from H-mode";
            break;
        case 0x0B:
            cause_str = "ECALL from M-mode";
            break;
        default:
            cause_str = "Unknown exception";
            break;
        }
    }

    uart_puts("\r\nTRAP: ");
    uart_puts(cause_str);
    uart_puts("\r\n  mcause = 0x");
    uart_put_hex64(mcause);
    uart_puts("\r\n  mepc   = 0x");
    uart_put_hex64(mepc);
    uart_puts("\r\n  mtval  = 0x");
    uart_put_hex64(mtval);
    uart_puts("\r\n");

    while (1) {
        __asm__ volatile("wfi");
    }
}
