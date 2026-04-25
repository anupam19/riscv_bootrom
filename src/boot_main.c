#include "boot.h"
#include "uart.h"

/* Optional FENCE.I self-modifying code test.
   Compile with -DENABLE_FENCE_I_TEST to run.
   This verifies that instruction fetches see prior stores after FENCE.I. */
#ifdef ENABLE_FENCE_I_TEST
static void test_fence_i(void)
{
    /* Code buffer in RAM (must be executable) */
    volatile uint32_t *code_buf = (volatile uint32_t *)0x80010000;
    uint32_t orig_instr = code_buf[0];
    uint32_t new_instr;

    /* Build a simple instruction: addi x0, x0, 0  (NOP) encoded as 0x00000013 */
    new_instr = 0x00000013;

    /* Store new instruction */
    code_buf[0] = new_instr;

    /* Ensure store completes before instruction fetch */
    fence_i();

    /* Execute from the modified location */
    void (*func)(void) = (void (*)(void))code_buf;
    func();

    /* Restore original instruction */
    code_buf[0] = orig_instr;
    fence_i();
}
#endif

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

#ifdef ENABLE_FENCE_I_TEST
    test_fence_i();
#endif

    /* Jump to next stage using full 32-bit address construction
       (LUI + ADDI + JALR). This demonstrates PIC-style jump
       without using a direct function call. */

    uintptr_t target = 0x80020000UL;

    __asm__ volatile("lui x10, %0\n\t"
                     "addi x10, x10, %1\n\t"
                     "jalr x0, x10, 0"
                     :
                     : "i"(target >> 12), "i"(target & 0xFFF)
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

#ifdef RV_E
    /* Optional: Illegal register access detection for RV32E/RV64E
       If the trap cause is illegal instruction, decode the faulting
       instruction to see whether it referenced a register >= 16. */
    if (mcause == 2) { /* CAUSE_ILLEGAL_INSTRUCTION */
        uint32_t fault_instr = *(volatile uint32_t *)mepc;
        uint32_t rs1 = (fault_instr >> 15) & 0x1F;
        uint32_t rs2 = (fault_instr >> 20) & 0x1F;
        uint32_t rd = (fault_instr >> 11) & 0x1F;
        if (rs1 >= 16 || rs2 >= 16 || rd >= 16) {
            uart_puts("  Illegal register access: ");
            if (rs1 >= 16) {
                uart_puts("rs1=x");
                uart_put_hex8(rs1);
                uart_puts(" ");
            }
            if (rs2 >= 16) {
                uart_puts("rs2=x");
                uart_put_hex8(rs2);
                uart_puts(" ");
            }
            if (rd >= 16) {
                uart_puts("rd=x");
                uart_put_hex8(rd);
                uart_puts(" ");
            }
            uart_puts("\r\n");
        }
    }
#endif

    while (1) {
        __asm__ volatile("wfi");
    }
}
