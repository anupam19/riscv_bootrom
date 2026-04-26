    .section .text
    .global _start
    .type _start, @function
_start:
    # UART base: 0x10000000 (lui sets upper 20 bits)
    lui t0, 0x10000          # t0 = 0x10000000

    # Load string address (absolute) into t1
    lui t1, %hi(string)
    addi t1, t1, %lo(string)

print_loop:
    lbu t2, 0(t1)            # load byte
    beqz t2, print_done      # null terminator -> done

    # Wait for UART THRE (bit 5 of LSR at offset 5)
wait_thre:
    lbu t3, 5(t0)
    andi t3, t3, 0x20
    beqz t3, wait_thre

    sb t2, 0(t0)             # write character
    fence w, w               # ensure commit

    addi t1, t1, 1
    j print_loop

print_done:
    # Simple delay loop: for (t4 = 0; t4 < 100000; t4++)
    li t4, 0
delay_loop:
    addi t4, t4, 1
    blt t4, 100000, delay_loop

    # Write test finisher to exit QEMU cleanly.
    # Device at 0x100000 (VIRT_TEST in QEMU virt).
    lui t5, 0x100            # t5 = 0x100000
    li t6, 0x5555            # FINISHER_PASS
    sh t6, 0(t5)             # 16-bit store
    fence w, w               # ensure write completes

hang:
    wfi
    j hang

    .section .rodata
    .align 2
string:
    .string "PAYLOAD: OK\r\n"
