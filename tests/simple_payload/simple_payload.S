    .section .text
    .global _start
    .type _start, @function
_start:
    # Write 0x5555 to test finisher (word store)
    lui t0, 0x100            # t0 = 0x100000
    li t1, 0x5555
    sw t1, 0(t0)
    fence w, w

hang:
    wfi
    j hang
