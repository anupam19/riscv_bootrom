#include "boot.h"
#include "timer.h"

/* Platform initialization - override per-platform if needed */
__attribute__((weak)) void platform_init(void)
{
    /* Optional: enable machine timer interrupt for testing.
       Compile with -DENABLE_TIMER_TEST to activate. */
#ifdef ENABLE_TIMER_TEST
    timer_init(1000000);  /* 1 second */
    csr_set(CSR_MIE, (1UL << 7)); /* Enable MTIE (machine timer interrupt) */
#endif
}


