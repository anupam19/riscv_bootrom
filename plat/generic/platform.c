#include "boot.h"

/* Platform initialization - override per-platform if needed */
__attribute__((weak)) void platform_init(void)
{
    /* No hardware init needed for generic platform */
    /* Real implementations would: clock enables, pinmux, DRAM init, etc. */
}
