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
    beq t0, t1, 2f
1:
    sw zero, 0(t0)
    addi t0, t0, 4
    blt t0, t1, 1b
2:

    /* Install trap handler (direct mode) */
    la t0, trap_entry
    csrw mtvec, t0

    /* Disable all interrupts (optional — reset state is MIE=0) */
    li t0, 0
    csrw mie, t0

    /* Jump to C entry point */
    call boot_main

    /* Should never return — infinite loop */
3:
    wfi
    j 3b

    .size _start, . - _start

/* Trap entry — all exceptions/interrupts vector here */
    .align 2
    .globl trap_entry
    .type trap_entry, @function
trap_entry:
    /* Minimal stack frame for ABI compliance (if we call C) */
    addi sp, sp, -16
    sw ra, 8(sp)          /* Save return address (for debugging) */

    /* Call C trap handler */
    call trap_handler

    /* trap_handler should not return, but safe infinite loop */
4:
    wfi
    j 4b

    .size trap_entry, . - trap_entry
