#ifndef BOOT_H
#define BOOT_H

#include <stdint.h>

void boot_main(void) __attribute__((noreturn));
void trap_handler(void) __attribute__((noreturn));
void platform_init(void);

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

#endif /* BOOT_H */
