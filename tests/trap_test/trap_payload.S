    .section .text
    .global _start
    .type _start, @function
_start:
    # UART base: 0x10000000
    lui t0, 0x10000          # t0 = 0x10000000

    # Load string address
    lui t1, %hi(string)
    addi t1, t1, %lo(string)

print_loop:
    lbu t2, 0(t1)
    beqz t2, print_done

    # Wait for THRE
wait_thre:
    lbu t3, 5(t0)
    andi t3, t3, 0x20
    beqz t3, wait_thre

    sb t2, 0(t0)
    fence w, w
    addi t1, t1, 1
    j print_loop

print_done:
    # Short delay
    li t4, 0
delay_loop:
    addi t4, t4, 1
    blt t4, 10000, delay_loop

    # Trigger ECALL (encoded 0x00000073)
    .word 0x00000073

    # Should not return
hang:
    wfi
    j hang

    .section .rodata
    .align 2
string:
    .string "TRAP_TEST: triggering ECALL...\r\n"
