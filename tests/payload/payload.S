    .section .text
    .global _start
    .type _start, @function
_start:
    # UART base: 0x10000000
    lui t0, 0x10000          # t0 = 0x10000000

    # Get address of string via PC-relative
    auipc t1, %pcrel_hi(string)
    addi t1, t1, %pcrel_lo(string)

print_loop:
    lbu t2, 0(t1)
    beqz t2, print_done

    # Wait for THRE (bit 5 of LSR at offset 5)
wait_thre:
    lbu t3, 5(t0)
    andi t3, t3, 0x20
    beqz t3, wait_thre

    sb t2, 0(t0)
    fence w, w

    addi t1, t1, 1
    j print_loop

print_done:
    # Delay loop: t4 = 0; while (t4 < 100000) t4++
    li t4, 0
    li t5, 100000
delay_loop:
    addi t4, t4, 1
    blt t4, t5, delay_loop

    # Test finisher write to exit QEMU
    lui t5, 0x100            # t5 = 0x100000
    li t6, 0x5555
    sh t6, 0(t5)             # halfword store
    fence w, w

hang:
    wfi
    j hang

    # String placed in text section
    .align 2
string:
    .string "PAYLOAD: OK\r\n"
