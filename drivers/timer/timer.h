#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

/* CLINT base address (SiFive standard) */
#define CLINT_BASE 0x02000000UL

/* Offsets within CLINT */
#define CLINT_MTIME_OFFSET   0x0000UL  /* read‑only 64‑bit time register */
#define CLINT_MTIMECMP_OFFSET 0x4000UL /* per‑hart compare register (64‑bit) */

/* Accessors — volatile for MMIO */
static inline uint64_t timer_get_time(void)
{
    volatile uint64_t *mtime = (volatile uint64_t *)(CLINT_BASE + CLINT_MTIME_OFFSET);
    return *mtime;
}

static inline void timer_set_compare(uint64_t val)
{
    volatile uint64_t *mtimecmp = (volatile uint64_t *)(CLINT_BASE + CLINT_MTIMECMP_OFFSET);
    *mtimecmp = val;
}

/* Initialize timer to fire after 'usec' microseconds */
void timer_init(uint64_t usec);

#endif /* TIMER_H */
