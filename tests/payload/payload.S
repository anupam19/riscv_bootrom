    .section .text
    .global _start
    .type _start, @function
_start:
    # UART base: 0x10000000
    lui t0, 0x10000

    # ---- print character in t1 ----
# macro: wait for THRE then store; use repeated block for each char
    li t1, 'P'
1:  lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 1b
    sb t1, 0(t0)
    fence w, w

    li t1, 'A'
2:  lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 2b
    sb t1, 0(t0)
    fence w, w

    li t1, 'Y'
3:  lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 3b
    sb t1, 0(t0)
    fence w, w

    li t1, 'L'
4:  lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 4b
    sb t1, 0(t0)
    fence w, w

    li t1, 'O'
5:  lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 5b
    sb t1, 0(t0)
    fence w, w

    li t1, 'A'
6:  lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 6b
    sb t1, 0(t0)
    fence w, w

    li t1, 'D'
7:  lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 7b
    sb t1, 0(t0)
    fence w, w

    li t1, ':'
8:  lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 8b
    sb t1, 0(t0)
    fence w, w

    li t1, ' '
9:  lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 9b
    sb t1, 0(t0)
    fence w, w

    li t1, 'O'
10: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 10b
    sb t1, 0(t0)
    fence w, w

    li t1, 'K'
11: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 11b
    sb t1, 0(t0)
    fence w, w

    li t1, '\r'
12: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 12b
    sb t1, 0(t0)
    fence w, w

    li t1, '\n'
13: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 13b
    sb t1, 0(t0)
    fence w, w

    # Delay loop
    li t3, 0
    li t4, 100000
delay_loop:
    addi t3, t3, 1
    blt t3, t4, delay_loop

    # Write test finisher (word store to 0x100000)
    lui t5, 0x100
    li t6, 0x5555
    sw t6, 0(t5)
    fence w, w

hang:
    wfi
    j hang
