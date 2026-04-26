/* Minimal test payload: immediately signal success via SiFive Test Finisher.
   No UART, no dependencies. Entry: _start at 0x80020000.
   Writes 0x5555 to test finisher to cause QEMU exit with code 0.
*/

#ifndef TEST_FINISHER_ADDR
#define TEST_FINISHER_ADDR 0x100000
#endif

#define TEST_FINISHER ((volatile unsigned int *)TEST_FINISHER_ADDR)

void _start(void)
{
    /* Write pass status (low 16 bits = 0x5555). The upper 16 bits can be zero for exit code 0 */
    *TEST_FINISHER = 0x5555;

    /* Should not reach here. */
    while (1) {
        __asm__ volatile("wfi");
    }
}
