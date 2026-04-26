#ifndef BITMANIP_H
#define BITMANIP_H

#include <stdint.h>

/* Bit manipulation primitives — Zb_{ba,bb,bs}
   These map to compiler built‑ins when available, otherwise use C fallbacks.
   All functions operate on 64‑bit values; RV32 truncates naturally. */

/* Count leading zeros (Zbb.clz) */
static inline uint64_t bit_clz(uint64_t x)
{
    if (x == 0) return 64;
#if defined(__has_builtin) && __has_builtin(__builtin_clzll)
    return (uint64_t)__builtin_clzll(x);
#else
    uint64_t count = 0;
    for (int i = 63; i >= 0; i--) {
        if (x & ((uint64_t)1 << i)) break;
        count++;
    }
    return count;
#endif
}

/* Count trailing zeros (Zbb.ctz) */
static inline uint64_t bit_ctz(uint64_t x)
{
    if (x == 0) return 64;
#if defined(__has_builtin) && __has_builtin(__builtin_ctzll)
    return (uint64_t)__builtin_ctzll(x);
#else
    uint64_t count = 0;
    for (int i = 0; i < 64; i++) {
        if (x & ((uint64_t)1 << i)) break;
        count++;
    }
    return count;
#endif
}

/* Population count (Zbb.cpop) */
static inline uint64_t bit_cpop(uint64_t x)
{
#if defined(__has_builtin) && __has_builtin(__builtin_popcountll)
    return (uint64_t)__builtin_popcountll(x);
#else
    uint64_t count = 0;
    while (x) {
        count += x & 1;
        x >>= 1;
    }
    return count;
#endif
}

/* Bit set (Zbs.bset) — set bit at position */
static inline uint64_t bit_bset(uint64_t x, unsigned int pos)
{
    if (pos >= 64) return x;
    return x | ((uint64_t)1 << pos);
}

/* Bit clear (Zbs.bclr) — clear bit at position */
static inline uint64_t bit_bclr(uint64_t x, unsigned int pos)
{
    if (pos >= 64) return x;
    return x & ~((uint64_t)1 << pos);
}

/* Bit invert (Zbs.binv) — invert bit at position */
static inline uint64_t bit_binv(uint64_t x, unsigned int pos)
{
    if (pos >= 64) return x;
    return x ^ ((uint64_t)1 << pos);
}

/* Bit extract (Zbs.bext) — extract bit at position (0 or 1) */
static inline uint64_t bit_bext(uint64_t x, unsigned int pos)
{
    if (pos >= 64) return 0;
    return (x >> pos) & 1;
}

#endif /* BITMANIP_H */
