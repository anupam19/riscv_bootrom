#include "plic.h"
#include "boot.h"
#include "uart.h"

/* Test external interrupt source number (e.g., UART RX) */
#define PLIC_TEST_SOURCE 1

void plic_init(void)
{
    /* Set priority for test source to medium (e.g., 3) */
    plic_set_priority(PLIC_TEST_SOURCE, 3);

    /* Enable this source for current hart (hart 0) */
    plic_enable(PLIC_TEST_SOURCE);

    /* FENCE after MMIO writes */
    __asm__ volatile("fence w, w" : : : "memory");
}
