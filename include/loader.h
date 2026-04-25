#ifndef LOADER_H
#define LOADER_H

#include <stdint.h>

/* Memory‑mapped SPI flash base (adjust per platform) */
#define FLASH_BASE 0x10000000UL

/* Default boot image offset in flash */
#define BOOT_IMAGE_OFFSET 0x0UL

/* Loader entry point type */
typedef void (*entry_fn)(void);

/* Load the boot image from flash and jump to it.
   Returns only on error (trap handler will halt). */
void load_and_jump(void);

#endif /* LOADER_H */
