/* Linker script for test payload
   Links at the BootROM handoff address: 0x80020000
   Minimal footprint — only .text section needed.
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

   .rodata :
   {
     *(.rodata .rodata.*)
   } > RAM
}
