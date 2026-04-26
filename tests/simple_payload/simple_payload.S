    .section .text
    .global _start
    .type _start, @function
_start:
    lui t0, 0x100          # t0 = 0x100000 (test finisher base)

    # Prepare value 0x5555 in t1
    li t1, 0x5555          # will expand to appropriate instructions

    # Halfword store
    sh t1, 0(t0)
    fence w, w

hang:
    wfi
    j hang
