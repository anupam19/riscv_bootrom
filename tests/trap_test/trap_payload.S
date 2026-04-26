    .section .text
    .global _start
    .type _start, @function
_start:
    lui t0, 0x10000          # UART base

    # Print "TRAP_TEST: triggering ECALL...\r\n"
    li t1, 'T'
1: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 1b
    sb t1, 0(t0)
    fence w, w

    li t1, 'R'
2: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 2b
    sb t1, 0(t0)
    fence w, w

    li t1, 'A'
3: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 3b
    sb t1, 0(t0)
    fence w, w

    li t1, 'P'
4: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 4b
    sb t1, 0(t0)
    fence w, w

    li t1, '_'
5: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 5b
    sb t1, 0(t0)
    fence w, w

    li t1, 'T'
6: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 6b
    sb t1, 0(t0)
    fence w, w

    li t1, 'E'
7: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 7b
    sb t1, 0(t0)
    fence w, w

    li t1, 'S'
8: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 8b
    sb t1, 0(t0)
    fence w, w

    li t1, 'T'
9: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 9b
    sb t1, 0(t0)
    fence w, w

    li t1, ':'
10: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 10b
    sb t1, 0(t0)
    fence w, w

    li t1, ' '
11: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 11b
    sb t1, 0(t0)
    fence w, w

    li t1, 't'
12: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 12b
    sb t1, 0(t0)
    fence w, w

    li t1, 'r'
13: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 13b
    sb t1, 0(t0)
    fence w, w

    li t1, 'i'
14: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 14b
    sb t1, 0(t0)
    fence w, w

    li t1, 'g'
15: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 15b
    sb t1, 0(t0)
    fence w, w

    li t1, 'g'
16: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 16b
    sb t1, 0(t0)
    fence w, w

    li t1, 'e'
17: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 17b
    sb t1, 0(t0)
    fence w, w

    li t1, 'r'
18: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 18b
    sb t1, 0(t0)
    fence w, w

    li t1, 'i'
19: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 19b
    sb t1, 0(t0)
    fence w, w

    li t1, 'n'
20: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 20b
    sb t1, 0(t0)
    fence w, w

    li t1, 'g'
21: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 21b
    sb t1, 0(t0)
    fence w, w

    li t1, ' '
22: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 22b
    sb t1, 0(t0)
    fence w, w

    li t1, 'E'
23: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 23b
    sb t1, 0(t0)
    fence w, w

    li t1, 'C'
24: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 24b
    sb t1, 0(t0)
    fence w, w

    li t1, 'A'
25: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 25b
    sb t1, 0(t0)
    fence w, w

    li t1, 'L'
26: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 26b
    sb t1, 0(t0)
    fence w, w

    li t1, 'L'
27: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 27b
    sb t1, 0(t0)
    fence w, w

    li t1, '.'
28: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 28b
    sb t1, 0(t0)
    fence w, w

    li t1, '.'
29: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 29b
    sb t1, 0(t0)
    fence w, w

    li t1, '.'
30: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 30b
    sb t1, 0(t0)
    fence w, w

    li t1, '\r'
31: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 31b
    sb t1, 0(t0)
    fence w, w

    li t1, '\n'
32: lbu t2, 5(t0)
    andi t2, t2, 0x20
    beqz t2, 32b
    sb t1, 0(t0)
    fence w, w

    # Small delay
    li t3, 0
    li t4, 10000
delay_loop:
    addi t3, t3, 1
    blt t3, t4, delay_loop

    # Trigger ECALL
    .word 0x00000073

hang:
    wfi
    j hang
