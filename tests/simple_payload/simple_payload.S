    .section .text
    .global _start
    .type _start, @function
_start:
    lui t0, 0x10000         # UART base 0x10000000
    li t1, 'X'              # character to output

    # Wait for THRE (bit 5 of LSR offset 5)
wait_thre:
    lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, wait_thre

    sb t1, 0(t0)            # write 'X'
    fence w, w

    # Small delay to allow UART to transmit
    li t3, 0
    li t4, 100000
1:
    addi t3, t3, 1
    blt t3, t4, 1b

    # Now write test finisher: address 0x100000 (VIRT_TEST)
    lui t0, 0x100           # t0 = 0x100000
    li t1, 0x5555
    sw t1, 0(t0)            # word store
    fence w, w

hang:
    wfi
    j hang
