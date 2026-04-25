#ifndef PLIC_H
#define PLIC_H

#include <stdint.h>

/* Simplified PLIC memory map (SiFive-style) */
#define PLIC_BASE 0x0C000000UL /* PLIC base address */

/* PLIC registers (per priority, pending, claim/complete) — simplified */
#define PLIC_PENDING_OFFSET 0x0000UL  /* 32-bit pending bits for each interrupt source */
#define PLIC_ENABLE_OFFSET 0x0080UL   /* Interrupt enable bits per hart/context */
#define PLIC_PRIORITY_OFFSET 0x0100UL /* Priority thresholds and claims */
#define PLIC_CLAIM_OFFSET 0x0200UL    /* Claim/complete registers */

/* Number of interrupt sources supported (e.g., 32) */
#define PLIC_NUM_SOURCES 32

/* Convert source number to register offset (each source has 4-byte priority) */
static inline uint32_t plic_priority_addr(uint32_t source)
{
    return PLIC_BASE + PLIC_PRIORITY_OFFSET + (source * 4);
}

/* Read the claim register for current hart (returns interrupt number or 0) */
static inline uint32_t plic_claim(void)
{
    volatile uint32_t *claim = (volatile uint32_t *)(PLIC_BASE + PLIC_CLAIM_OFFSET);
    return *claim;
}

/* Write to claim register to signal completion (same address) */
static inline void plic_complete(uint32_t source)
{
    volatile uint32_t *claim = (volatile uint32_t *)(PLIC_BASE + PLIC_CLAIM_OFFSET);
    *claim = source;
}

/* Set enable bit for a specific source (hart-specific context) */
static inline void plic_enable(uint32_t source)
{
    volatile uint32_t *enable = (volatile uint32_t *)(PLIC_BASE + PLIC_ENABLE_OFFSET);
    *enable |= (1U << source);
}

/* Clear enable bit for a source */
static inline void plic_disable(uint32_t source)
{
    volatile uint32_t *enable = (volatile uint32_t *)(PLIC_BASE + PLIC_ENABLE_OFFSET);
    *enable &= ~(1U << source);
}

/* Set priority for a source (0 = disabled, higher = more urgent) */
static inline void plic_set_priority(uint32_t source, uint32_t prio)
{
    volatile uint32_t *addr = (volatile uint32_t *)plic_priority_addr(source);
    *addr = prio & 0x7; /* 3-bit priority */
}

/* Initialize PLIC: enable a test source (e.g., source 1 = UART) */
void plic_init(void);

#endif /* PLIC_H */
