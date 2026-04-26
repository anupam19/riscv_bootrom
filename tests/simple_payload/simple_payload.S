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

    # Now write test finisher
    lui t0, 0x1000          # t0 = 0x1000000 (VIRT_TEST base)
    li t1, 0x5555
    sw t1, 0(t0)            # word store
    fence w, w

hang:
    wfi
    j hang
