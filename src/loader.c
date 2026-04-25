#include "loader.h"
#include "boot.h"
#include "uart.h"

/* Destination in RAM where the image will be loaded */
#define LOAD_ADDR 0x80020000UL

/* Size of the image to load (bytes) — must match the built image size */
#define IMAGE_MAX_SIZE 0x40000UL /* 256 KB */

void load_and_jump(void)
{
    uart_puts("Loader: Starting image load from flash...\r\n");

    volatile uint8_t *src = (volatile uint8_t *)FLASH_BASE;
    volatile uint8_t *dst = (volatile uint8_t *)LOAD_ADDR;
    uint32_t remaining = IMAGE_MAX_SIZE;
    uint32_t crc = 0;

    while (remaining--) {
        uint8_t byte = *src;
        *dst = byte;
        /* Simple CRC‑32 (placeholder) */
        crc ^= byte << 24;
        src++;
        dst++;
    }

    uart_puts("Loader: Image loaded to 0x");
    uart_put_hex64(LOAD_ADDR);
    uart_puts("\r\n");

    /* Optional: verify CRC stored at end of image? (skip for now) */

    /* Jump to entry point (first word = entry address) */
    entry_fn entry = (entry_fn)LOAD_ADDR;
    uart_puts("Loader: Jumping to entry...\r\n");

    /* Disable interrupts and jump */
    csr_write(CSR_MTVEC, 0); /* clear mtvec to avoid accidental traps in loaded code */
    entry();
}
