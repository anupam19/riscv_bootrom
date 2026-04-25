OUTPUT_FORMAT("elf64-littleriscv")
ENTRY(_start)

MEMORY
{
  ROM (rx) : ORIGIN = 0x1000, LENGTH = 256K
  RAM (rwx) : ORIGIN = 0x80000000, LENGTH = 16M
}

SECTIONS
{
  .text :
  {
    . = ALIGN(4);
    _start = .;
    KEEP(*(.text.boot))
    *(.text .text.*)
    . = ALIGN(4);
  } > ROM

  .rodata :
  {
    *(.rodata .rodata.*)
    . = ALIGN(4);
  } > ROM

  .data :
  {
    *(.data .data.*)
    . = ALIGN(4);
  } > ROM

  .bss :
  {
    __bss_start = .;
    *(.bss .bss.*)
    *(COMMON)
    __bss_end = .;
    . = ALIGN(4);
  } > RAM

  _stack_top = ORIGIN(RAM) + LENGTH(RAM);
  __stack_start = .;
}
