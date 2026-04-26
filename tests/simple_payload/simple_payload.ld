/* Linker script for simple test payload
   Links at 0x80020000.
*/

OUTPUT_FORMAT("elf64-littleriscv")
ENTRY(_start)

MEMORY
{
  RAM (rwx) : ORIGIN = 0x80020000, LENGTH = 1M
}

SECTIONS
{
  .text :
  {
    . = ALIGN(4);
    _start = .;
    *(.text .text.*)
    . = ALIGN(4);
  } > RAM
}
