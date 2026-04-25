#include "boot.h"
#include "timer.h"
#ifdef ENABLE_PLIC_TEST
#include "plic.h"
#endif

/* Platform initialization - override per-platform if needed */
__attribute__((weak)) void platform_init(void)
{
    /* Optional: enable machine timer interrupt for testing.
       Compile with -DENABLE_TIMER_TEST to activate. */
#ifdef ENABLE_TIMER_TEST
    timer_init(1000000);          /* 1 second */
    csr_set(CSR_MIE, (1UL << 7)); /* MTIE */
#endif

#ifdef ENABLE_PLIC_TEST
    plic_init();                   /* Initialize PLIC and enable test source */
    csr_set(CSR_MIE, (1UL << 11)); /* MEIE — machine external interrupt enable */
#endif
}
