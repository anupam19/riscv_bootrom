    .section .text.boot
    .globl _start
    .type _start, @function
_start:
    /* Set up stack pointer */
    .option push
    .option norelax
    la sp, _stack_top
    .option pop

    /* Zero .bss section */
    la t0, __bss_start
    la t1, __bss_end
    beq t0, t1, 2f      /* Skip if bss empty */
1:
    sw zero, 0(t0)
    addi t0, t0, 4
    blt t0, t1, 1b
2:

    /* Jump to C entry point */
    call boot_main

    /* Should never return - infinite loop */
3:
    wfi
    j 3b

    .size _start, . - _start
