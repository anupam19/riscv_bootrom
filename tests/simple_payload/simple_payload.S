    .section .text
    .global _start
    .type _start, @function
_start:
    lui t0, 0x100            # t0 = 0x100000 (test finisher)

    li t1, 0x5555
    sw t1, 0(t0)             # word store
    fence w, w

hang:
    wfi
    j hang
