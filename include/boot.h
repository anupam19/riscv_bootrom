#ifndef BOOT_H
#define BOOT_H

#include <stdint.h>

#include "uart.h"

void boot_main(void) __attribute__((noreturn));
void trap_handler(void) __attribute__((noreturn));
void platform_init(void);

/* CSR execution — for emulator / core */
void exec_csr(uint32_t instr);

extern uintptr_t _start;
extern uintptr_t _stack_top;
extern uintptr_t __bss_start;
extern uintptr_t __bss_end;

typedef uintptr_t addr_t;

#define BOOT_OK 0
#define BOOT_ERR -1

/* CSR addresses */
#define CSR_MSTATUS 0x300
#define CSR_MIE 0x304
#define CSR_MTVEC 0x305
#define CSR_MEPC 0x341
#define CSR_MCAUSE 0x342
#define CSR_MTVAL 0x343

/* Machine trap cause codes */
#define CAUSE_MACHINE_SOFTWARE_INTERRUPT 0x80000001
#define CAUSE_MACHINE_TIMER_INTERRUPT 0x80000007
#define CAUSE_MACHINE_EXTERNAL_INTERRUPT 0x8000000B
#define CAUSE_USER_ECALL 0x00000008
#define CAUSE_SUPERVISOR_ECALL 0x00000009
#define CAUSE_HYPERVISOR_ECALL 0x0000000A
#define CAUSE_MACHINE_ECALL 0x0000000B
#define CAUSE_INSTRUCTION_PAGE_FAULT 0x0000000C
#define CAUSE_LOAD_PAGE_FAULT 0x0000000D
#define CAUSE_STORE_PAGE_FAULT 0x0000000F
#define CAUSE_INSTRUCTION_ACCESS_FAULT 0x00000001
#define CAUSE_LOAD_ACCESS_FAULT 0x00000004
#define CAUSE_STORE_ACCESS_FAULT 0x00000006
#define CAUSE_ILLEGAL_INSTRUCTION 0x00000002
#define CAUSE_BREAKPOINT 0x00000003
#define CAUSE_MISALIGNED_FETCH 0x00000000
#define CAUSE_MISALIGNED_LOAD 0x00000004
#define CAUSE_MISALIGNED_STORE 0x00000006

/* CSR access helpers */
static inline unsigned long csr_read(unsigned long csr)
{
    unsigned long val;
    __asm__ volatile("csrr %0, %1" : "=r"(val) : "i"(csr));
    return val;
}

static inline void csr_write(unsigned long csr, unsigned long val)
{
    __asm__ volatile("csrw %0, %1" : : "i"(csr), "r"(val));
}

/* CSR set: read-modify-write with OR (set bits) */
static inline void csr_set(unsigned long csr, unsigned long mask)
{
    __asm__ volatile("csrs %0, %1" : : "i"(csr), "r"(mask));
}

/* CSR clear: read-modify-write with AND NOT (clear bits) */
static inline void csr_clear(unsigned long csr, unsigned long mask)
{
    __asm__ volatile("csrc %0, %1" : : "i"(csr), "r"(mask));
}

/* Zicsr: atomic read-modify-write CSR instructions (return previous value) */

/* CSRRW — atomic swap: old = CSR; CSR = rs1; return old */
static inline uint64_t csr_csrrw(uint16_t csr, uint64_t val)
{
    uint64_t old;
    __asm__ volatile("csrrw %0, %1, %2" : "=r"(old) : "i"(csr), "r"(val));
    return old;
}

/* CSRRS — atomic set: old = CSR; CSR = old | rs1; return old.
   Special: if rs1 == 0 → read only, no write (use x0 source). */
static inline uint64_t csr_csrrs(uint16_t csr, uint64_t mask)
{
    uint64_t old;
    if (mask == 0) {
        __asm__ volatile("csrrs %0, %1, x0" : "=r"(old) : "i"(csr));
    } else {
        __asm__ volatile("csrrs %0, %1, %2" : "=r"(old) : "i"(csr), "r"(mask));
    }
    return old;
}

/* CSRRC — atomic clear: old = CSR; CSR = old & ~rs1; return old.
   Special: if rs1 == 0 → read only, no write. */
static inline uint64_t csr_csrrc(uint16_t csr, uint64_t mask)
{
    uint64_t old;
    if (mask == 0) {
        __asm__ volatile("csrrc %0, %1, x0" : "=r"(old) : "i"(csr));
    } else {
        __asm__ volatile("csrrc %0, %1, %2" : "=r"(old) : "i"(csr), "r"(mask));
    }
    return old;
}

/* Immediate variants (5-bit zero-extended immediate) */

/* CSRRWI — write zero-extended immediate, return old */
static inline uint64_t csr_csrrwi(uint16_t csr, uint8_t uimm)
{
    uint64_t old;
    __asm__ volatile("csrrwi %0, %1, %2" : "=r"(old) : "i"(csr), "i"(uimm));
    return old;
}

/* CSRRSI — atomic set with immediate, return old.
   Special: if uimm == 0 → read only. */
static inline uint64_t csr_csrrsi(uint16_t csr, uint8_t uimm)
{
    uint64_t old;
    if (uimm == 0) {
        __asm__ volatile("csrrs %0, %1, x0" : "=r"(old) : "i"(csr));
    } else {
        __asm__ volatile("csrrsi %0, %1, %2" : "=r"(old) : "i"(csr), "i"(uimm));
    }
    return old;
}

/* CSRRCI — atomic clear with immediate, return old.
   Special: if uimm == 0 → read only. */
static inline uint64_t csr_csrrci(uint16_t csr, uint8_t uimm)
{
    uint64_t old;
    if (uimm == 0) {
        __asm__ volatile("csrrc %0, %1, x0" : "=r"(old) : "i"(csr));
    } else {
        __asm__ volatile("csrrci %0, %1, %2" : "=r"(old) : "i"(csr), "i"(uimm));
    }
    return old;
}

/* Emulator core function: execute one CSR instruction (software model) */
#ifdef EMULATOR
void exec_csr(uint32_t instr);
#endif

/* Interrupt enable/disable (machine-level) */
static inline void enable_irq(void)
{
    csr_set(CSR_MIE, 0xFFFFUL); /* Enable all standard machine interrupts */
}

static inline void disable_irq(void)
{
    csr_clear(CSR_MIE, 0xFFFFUL);
}

/* Set MTVEC to direct or vectored mode */
/* mode=0: direct (BASE & ~3), mode=1: vectored (BASE & ~3, +4*exception) */
static inline void set_mtvec(uintptr_t base, unsigned long mode)
{
    /* mode bit is bit 0 of the register value */
    csr_write(CSR_MTVEC, base | (mode & 0x1));
}

/* Read/write mstatus (global interrupts enable, previous mode, etc.) */
static inline unsigned long mstatus_read(void)
{
    return csr_read(CSR_MSTATUS);
}

static inline void mstatus_write(unsigned long val)
{
    csr_write(CSR_MSTATUS, val);
}

/* Set MIE (global interrupt enable) bit in mstatus (bit 3) */
static inline void mstatus_irq_enable(void)
{
    csr_set(CSR_MSTATUS, (1UL << 3)); /* Set MIE */
}

static inline void mstatus_irq_disable(void)
{
    csr_clear(CSR_MSTATUS, (1UL << 3)); /* Clear MIE */
}

#ifndef ENABLE_UART_DEBUG
#define ENABLE_UART_DEBUG 1
#endif

/* Misaligned access guard functions */
static inline uint64_t guarded_lb(volatile uint8_t *addr)
{
    return (uint64_t)(int8_t)(*addr); /* sign-extended byte */
}

static inline uint64_t guarded_lh(volatile uint16_t *addr)
{
    if ((uintptr_t)addr & 0x1) {
        uart_puts("Misaligned halfword load at 0x");
        uart_put_hex64((uintptr_t)addr);
        while (1) {
            __asm__ volatile("wfi");
        }
    }
    uint16_t val = *(volatile uint16_t *)addr;
    return (uint64_t)(int16_t)val; /* sign-extend */
}

static inline uint64_t guarded_lw(volatile uint32_t *addr)
{
    if ((uintptr_t)addr & 0x3) {
        uart_puts("Misaligned word load at 0x");
        uart_put_hex64((uintptr_t)addr);
        while (1) {
            __asm__ volatile("wfi");
        }
    }
    uint32_t val = *(volatile uint32_t *)addr;
    return (uint64_t)(int32_t)val; /* sign-extend to 64-bit */
}

static inline void guarded_sb(volatile uint8_t *addr, uint64_t val)
{
    *addr = (uint8_t)val;
}

static inline void guarded_sh(volatile uint16_t *addr, uint64_t val)
{
    if ((uintptr_t)addr & 0x1) {
        uart_puts("Misaligned halfword store at 0x");
        uart_put_hex64((uintptr_t)addr);
        while (1) {
            __asm__ volatile("wfi");
        }
    }
    *(volatile uint16_t *)addr = (uint16_t)val;
}

static inline void guarded_sw(volatile uint32_t *addr, uint64_t val)
{
    if ((uintptr_t)addr & 0x3) {
        uart_puts("Misaligned word store at 0x");
        uart_put_hex64((uintptr_t)addr);
        while (1) {
            __asm__ volatile("wfi");
        }
    }
    *(volatile uint32_t *)addr = (uint32_t)val; /* truncate */
}

/* RV64: guarded doubleword (64-bit) load/store */
#if __riscv_xlen == 64
static inline uint64_t guarded_ld(volatile uint64_t *addr)
{
    if ((uintptr_t)addr & 0x7) {
        uart_puts("Misaligned doubleword load at 0x");
        uart_put_hex64((uintptr_t)addr);
        while (1) {
            __asm__ volatile("wfi");
        }
    }
    return *(volatile uint64_t *)addr;
}

static inline void guarded_sd(volatile uint64_t *addr, uint64_t val)
{
    if ((uintptr_t)addr & 0x7) {
        uart_puts("Misaligned doubleword store at 0x");
        uart_put_hex64((uintptr_t)addr);
        while (1) {
            __asm__ volatile("wfi");
        }
    }
    *(volatile uint64_t *)addr = val;
}
#endif /* __riscv_xlen == 64 */

/* Zifencei: FENCE.I instruction — instruction cache flush / pipeline sync */
#if __riscv_xlen == 64
static inline void fence_i(void)
{
    /* FENCE.I — ensures following instruction fetches observe prior stores */
    __asm__ volatile("fence.i" : : : "memory");
}
#else
static inline void fence_i(void)
{
    /* On RV32, fence.i is also available in the I-extension (not E) */
    __asm__ volatile("fence.i" : : : "memory");
}
#endif

/* Emulator core function: execute one CSR instruction (software model) */
#ifdef EMULATOR
void exec_csr(uint32_t instr);
#endif

#endif /* BOOT_H */
