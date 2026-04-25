#ifdef EMULATOR

#include "boot.h"
#include <stdint.h>

/* Simulated CSR file for emulator / software model */
static uint64_t csr_file[4096] = {0};

/* Global GPR array for emulator context */
uint64_t gpr[32] = {0};

/* Minimal set of implemented CSRs (machine-mode, plus some counters) */
static int csr_implemented(uint16_t addr)
{
    switch (addr) {
    case 0x300:
    case 0x304:
    case 0x305:
    case 0x341:
    case 0x342:
    case 0x343:
    case 0x344:
    case 0xC00:
    case 0xC01:
    case 0xC80:
    case 0xC81:
        return 1;
    default:
        return 0;
    }
}

/* Writable CSRs — read-only counters excluded */
static int csr_writable(uint16_t addr)
{
    switch (addr) {
    case 0xC00:
    case 0xC01:
    case 0xC80:
    case 0xC81:
        return 0; /* read-only */
    default:
        return 1;
    }
}

/* Illegal instruction trap for emulator */
static void emu_trap_illegal(void)
{
    uart_puts("Illegal CSR instruction trap\r\n");
    while (1) {
        __asm__ volatile("wfi");
    }
}

/* Execute one CSR instruction (software model) */
void exec_csr(uint32_t instr)
{
    uint16_t csr = (instr >> 20) & 0xFFF;
    uint8_t funct3 = (instr >> 12) & 0x7;
    uint8_t rd = (instr >> 7) & 0x1F;
    uint8_t rs1 = (instr >> 15) & 0x1F;
    uint8_t uimm = rs1 & 0x1F;
    uint64_t old, val;

    if (!csr_implemented(csr)) {
        emu_trap_illegal();
        return;
    }

    old = csr_file[csr];

    if (rd != 0) {
        gpr[rd] = old;
    }

    int will_write = 0;
    switch (funct3) {
    case 0x1:
        will_write = 1;
        val = (rs1 == 0) ? 0 : gpr[rs1];
        break;
    case 0x2:
        will_write = (rs1 != 0);
        if (rs1 != 0)
            val = gpr[rs1];
        break;
    case 0x3:
        will_write = (rs1 != 0);
        if (rs1 != 0)
            val = gpr[rs1];
        break;
    case 0x5:
        will_write = 1;
        val = (uint64_t)uimm;
        break;
    case 0x6: /* CSRRSI */
        will_write = (uimm != 0);
        if (uimm != 0)
            val = (uint64_t)uimm;
        break;
    case 0x7: /* CSRRCI */
        will_write = (uimm != 0);
        if (uimm != 0)
            val = (uint64_t)uimm;
        break;
    default:
        emu_trap_illegal();
        return;
    }

    if (will_write && !csr_writable(csr)) {
        emu_trap_illegal();
        return;
    }

    if (will_write) {
        switch (funct3) {
        case 0x1:
            csr_file[csr] = val;
            break;
        case 0x2:
            csr_file[csr] = old | val;
            break;
        case 0x3:
            csr_file[csr] = old & ~val;
            break;
        case 0x5:
            csr_file[csr] = val;
            break;
        case 0x6:
            csr_file[csr] = old | val;
            break;
        case 0x7:
            csr_file[csr] = old & ~val;
            break;
        }
    }
}

#endif /* EMULATOR */