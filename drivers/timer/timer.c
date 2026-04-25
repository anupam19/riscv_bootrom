#include "timer.h"
#include "boot.h"
#include "uart.h"

/* Timer tick frequency — CLINT runs at CPU frequency; approximate 1 MHz for simplicity.
   In real hardware, this would be the CPU clock (e.g., 32 MHz). */
#define TIMER_TICK_HZ 1000000UL /* 1 MHz */

void timer_init(uint64_t usec)
{
    uint64_t now = timer_get_time();

    /* Set mtimecmp = now + (usec * TIMER_TICK_HZ / 1_000_000) */
    uint64_t ticks = usec * (TIMER_TICK_HZ / 1000000);
    uint64_t target = now + ticks;

    /* Ensure write ordering for MMIO */
    timer_set_compare(target);

    /* FENCE.W ensures the store completes before any subsequent memory ops */
    __asm__ volatile("fence w, w" : : : "memory");
}
