    .section .text
    .global _start
    .type _start, @function
_start:
    lui t0, 0x10000          # UART base

    # Load absolute address of string (hi20 + lo12)
    lui t1, %hi(string)
    addi t1, t1, %lo(string)

print_loop:
    lbu t2, 0(t1)
    beqz t2, print_done

wait_thre:
    lbu t3, 5(t0)
    andi t3, t3, 0x20
    beqz t3, wait_thre

    sb t2, 0(t0)
    fence w, w
    addi t1, t1, 1
    j print_loop

print_done:
    li t4, 0
    li t5, 10000
delay_loop:
    addi t4, t4, 1
    blt t4, t5, delay_loop

    .word 0x00000073   # ECALL

hang:
    wfi
    j hang

    .section .rodata
    .align 2
string:
    .string "TRAP_TEST: triggering ECALL...\r\n"
