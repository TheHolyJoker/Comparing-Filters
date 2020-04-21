//
// Created by tomer on 10/24/19.
//

#include <cassert>
#include "bit_op.h"

unsigned int msb32(register unsigned int x) {
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    return (x & ~(x >> 1));
}

unsigned char msb8(register __uint8_t x) {
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    return (x & ~(x >> 1));
}

unsigned int reverse(register unsigned int x) {
    x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
    x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
    x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
    x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
    return ((x >> 16) | (x << 16));

}

unsigned int floor_log2(register unsigned int x) {
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
#ifdef    LOG0UNDEFINED
    return(ones32(x) - 1);
#else
    return (ones32(x >> 1));
#endif
}

unsigned int ones32(register unsigned int x) {
    /* 32-bit recursive reduction using SWAR...
   but first step is mapping 2-bit values
   into sum of 2 1-bit values in sneaky way
*/
    x -= ((x >> 1) & 0x55555555);
    x = (((x >> 2) & 0x33333333) + (x & 0x33333333));
    x = (((x >> 4) + x) & 0x0f0f0f0f);
    x += (x >> 8);
    x += (x >> 16);
    return (x & 0x0000003f);
}

uint32_t select_r(uint64_t slot, uint32_t rank) {
//    uint64_t slot;          // Input value to find position with rank rank.
//    unsigned int rank;      // Input: bit's desired rank [1-64].
    unsigned int s;      // Output: Resulting position of bit with the desired rank.[1-64]
    uint64_t a, b, c, d; // Intermediate temporaries for bit count.
    unsigned int t;      // Bit count temporary.

    // Do a normal parallel bit count for a 64-bit integer,
    // but store all intermediate steps.
    // a = (slot & 0x5555...) + ((slot >> 1) & 0x5555...);
    a = slot - ((slot >> 1) & ~0UL / 3);
    // b = (a & 0x3333...) + ((a >> 2) & 0x3333...);
    b = (a & ~0UL / 5) + ((a >> 2) & ~0UL / 5);
    // c = (b & 0x0f0f...) + ((b >> 4) & 0x0f0f...);
    c = (b + (b >> 4)) & ~0UL / 0x11;
    // d = (c & 0x00ff...) + ((c >> 8) & 0x00ff...);
    d = (c + (c >> 8)) & ~0UL / 0x101;
    t = (d >> 32) + (d >> 48);
    // Now do branchless select!
    s = 64;
    // if (rank > t) {s -= 32; rank -= t;}
    s -= ((t - rank) & 256) >> 3;
    rank -= (t & ((t - rank) >> 8));
    t = (d >> (s - 16)) & 0xff;
    // if (rank > t) {s -= 16; rank -= t;}
    s -= ((t - rank) & 256) >> 4;
    rank -= (t & ((t - rank) >> 8));
    t = (c >> (s - 8)) & 0xf;
    // if (rank > t) {s -= 8; rank -= t;}
    s -= ((t - rank) & 256) >> 5;
    rank -= (t & ((t - rank) >> 8));
    t = (b >> (s - 4)) & 0x7;
    // if (rank > t) {s -= 4; rank -= t;}
    s -= ((t - rank) & 256) >> 6;
    rank -= (t & ((t - rank) >> 8));
    t = (a >> (s - 2)) & 0x3;
    // if (rank > t) {s -= 2; rank -= t;}
    s -= ((t - rank) & 256) >> 7;
    rank -= (t & ((t - rank) >> 8));
    t = (slot >> (s - 1)) & 0x1;
    // if (rank > t) s--;
    s -= ((t - rank) & 256) >> 8;
    //current res between [0,63]
    return 64 - s;
}

uint64_t convert_32_to_64(uint32_t slot) {
    return ((ulong) (slot) << 32ul) | 4294967295ul;
}

bool is_bit_rank_valid(uint64_t slot, uint32_t rank, uint32_t res) {
    return res || (slot & (1ULL << 63ul));
}

uint32_t my_bit_rank(uint64_t slot, uint32_t rank) {
    const size_t lim = sizeof(slot) * CHAR_BIT;
    ulong b = 1ULL << 63ul, count = 0;
    for (size_t i = 0; i < lim; ++i) {
        if (b & slot) { if (++count == rank) return i; }
        b >>= 1ul;
    }
    return 64;
}

uint64_t count(uint64_t slot, unsigned int bit_lim) {
//    uint64_t slot;       // Compute the rank (bits set) in slot from the MSB to bit_lim.
//    unsigned int bit_lim; // Bit position to count bits upto.
    uint64_t r = slot;       // Resulting rank of bit at bit_lim goes here.

    // Shift out bits after given position.
//    r = slot >> (sizeof(slot) * CHAR_BIT - bit_lim);
    // Count set bits in parallel.
    // r = (r & 0x5555...) + ((r >> 1) & 0x5555...);
    r = r - ((r >> 1) & ~0UL / 3);
    // r = (r & 0x3333...) + ((r >> 2) & 0x3333...);
    r = (r & ~0UL / 5) + ((r >> 2) & ~0UL / 5);
    // r = (r & 0x0f0f...) + ((r >> 4) & 0x0f0f...);
    r = (r + (r >> 4)) & ~0UL / 17;
    // r = r % 255;
    return (r * (~0UL / 255)) >> ((sizeof(slot) - 1) * CHAR_BIT);
}

/*
uint32_t bit_count(uint32_t v) {
    v = v - ((v >> 1) & 0x55555555);                    // reuse input as temporary
    v = (v & 0x33333333) + ((v >> 2) & 0x33333333);     // temp
    return ((v + (v >> 4) & 0xF0F0F0F) * 0x1010101) >> 24; // count

}
*/

uint32_t my_count(uint32_t slot) {
    size_t lim = sizeof(slot) * CHAR_BIT;
    assert(lim == 32);
    ulong b = 1ULL << 31ul;
    size_t res = 0;
    for (size_t i = 0; i < lim; ++i) {
        if (b & slot) res++;
        b >>= 1ul;
    }
    return res;
}

size_t array_zero_count(const uint32_t *a, size_t a_size) {
    size_t res = 0;
    for (size_t i = 0; i < a_size; ++i) res += bit_count(~a[i]);
    return res;
}

unsigned int naive_msb32(unsigned int x) {
    if (x == 0) return 0;
    unsigned int b = (-1);
    unsigned int counter = 31;
    for (int i = 0; i < 32; ++i) {
        if (b & x) return 31 - i;
        b >>= 1;
    }
    return 0;

}

/* Find the index of the n-th 1-bit in mask
       The index of the least significant bit is 0
       Return -1 if there is no such bit
    */
int find_nth_set_bit(uint32_t mask, int n) {
    int t, i = n, r = 0;
    uint32_t c1 = mask;
    uint32_t c2 = c1 - ((c1 >> 1) & 0x55555555);
    uint32_t c4 = ((c2 >> 2) & 0x33333333) + (c2 & 0x33333333);
    uint32_t c8 = ((c4 >> 4) + c4) & 0x0f0f0f0f;
    uint32_t c16 = ((c8 >> 8) + c8);
    int c32 = (int) (((c16 >> 16) + c16) & 0x3f);
    t = (c16) & 0x1f;
    if (i >= t) {
        r += 16;
        i -= t;
    }
    t = (c8 >> r) & 0x0f;
    if (i >= t) {
        r += 8;
        i -= t;
    }
    t = (c4 >> r) & 0x07;
    if (i >= t) {
        r += 4;
        i -= t;
    }
    t = (c2 >> r) & 0x03;
    if (i >= t) {
        r += 2;
        i -= t;
    }
    t = (c1 >> r) & 0x01;
    if (i >= t) { r += 1; }
    if (n >= c32) r = -1;
    return r;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

