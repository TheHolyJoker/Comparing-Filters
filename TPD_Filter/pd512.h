/*
 * Taken from this repository.
 * https://github.com/jbapple/crate-dictionary
 * */

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>


//#include "immintrin.h"
#include "x86intrin.h"

// returns the position (starting from 0) of the jth set bit of x.
inline uint64_t select64(uint64_t x, int64_t j) {
    assert(j < 64);
    const uint64_t y = _pdep_u64(UINT64_C(1) << j, x);
    return _tzcnt_u64(y);
}

// returns the position (starting from 0) of the jth set bit of x.
inline uint64_t select128(unsigned __int128 x, int64_t j) {
    const int64_t pop = _mm_popcnt_u64(x);
    if (j < pop) return select64(x, j);
    return 64 + select64(x >> 64, j - pop);
}

// returns the number of zeros before the jth (counting from 0) set bit of x
inline uint64_t nth64(uint64_t x, int64_t j) {
    const uint64_t y = select64(x, j);
    assert(y < 64);
    const uint64_t z = x & ((UINT64_C(1) << y) - 1);
    return y - _mm_popcnt_u64(z);
}

// returns the number of zeros before the jth (counting from 0) set bit of x
inline uint64_t nth128(unsigned __int128 x, int64_t j) {
    const uint64_t y = select128(x, j);
    assert(y < 128);
    const unsigned __int128 z = x & ((((unsigned __int128) 1) << y) - 1);
    return y - _mm_popcnt_u64(z) - _mm_popcnt_u64(z >> 64);
}

int popcount64(uint64_t x) {
    return _mm_popcnt_u64(x);
}

int popcount128(unsigned __int128 x) {
    const uint64_t hi = x >> 64;
    const uint64_t lo = x;
    return popcount64(lo) + popcount64(hi);
}

// find an 8-bit value in a pocket dictionary with quotients in [0,64) and 49 values
inline bool pd_find_64(int64_t quot, char rem, const __m512i *pd) {
    assert(quot < 64);
    // The header has size 64 + 49
    uint64_t header[2];
    // We need to copy (64+49) bits, but we copy slightly more and mask out the ones we
    // don't care about.
    //
    // memcpy is the only defined punning operation
    memcpy(header, pd, 16);
    header[1] = header[1] & ((UINT64_C(1) << 49) - 1);
    assert(_mm_popcnt_u64(header[0]) + _mm_popcnt_u64(header[1]) == 64);
    // [begin,end) is the indexes of the values that have quot as their quotient. the range
    // is exclusive and end is at most 49
    const int64_t p = _mm_popcnt_u64(header[0]);
    uint64_t begin = 0;
    if (quot > 0) {
        begin = ((quot <= p) ? nth64(header[0], quot - 1)
                             : ((64 - p) + nth64(header[1], quot - 1 - p)));
    }
    const uint64_t end =
            (quot < p) ? nth64(header[0], quot) : ((64 - p) + nth64(header[1], quot - p));
    assert(begin <= end);
    assert(end <= 49);
    const __m512i target = _mm512_set1_epi8(rem);
    uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd);
    // round up to remove the header
    v = v >> ((64 + 49 + CHAR_BIT - 1) / CHAR_BIT);
    return (v & ((UINT64_C(1) << end) - 1)) >> begin;
}

// find an 8-bit value in a pocket dictionary with quotients in [0,50) and 51 values
inline bool pd_find_50(int64_t quot, char rem, const __m512i *pd) {
    assert(quot < 50);
    // The header has size 50 + 51
    uint64_t header[2];
    // We need to copy (50+51) bits, but we copy slightly more and mask out the ones we
    // don't care about.
    //
    // memcpy is the only defined punning operation
    memcpy(header, pd, 13);
    // Requires little-endianness
    header[1] = header[1] & ((UINT64_C(1) << (50 + 51 - 64)) - 1);
    assert(_mm_popcnt_u64(header[0]) + _mm_popcnt_u64(header[1]) == 50);
    // [begin,end) is the indexes of the values that have quot as their quotient. the range
    // is exclusive and end is at most 51
    const int64_t p = _mm_popcnt_u64(header[0]);
    uint64_t begin = 0;
    if (quot > 0) {
        begin = ((quot <= p) ? nth64(header[0], quot - 1)
                             : ((64 - p) + nth64(header[1], quot - 1 - p)));
    }
    const uint64_t end =
            (quot < p) ? nth64(header[0], quot) : ((64 - p) + nth64(header[1], quot - p));
    assert(begin <= end);
    assert(end <= 51);
    const __m512i target = _mm512_set1_epi8(rem);
    uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd);
    // round up to remove the header
    v = v >> ((51 + 50 + CHAR_BIT - 1) / CHAR_BIT);
    return (v & ((UINT64_C(1) << end) - 1)) >> begin;
}


// insert a pair of a quotient (mod 50) and an 8-bit remainder in a pocket dictionary.
// Returns false if the dictionary is full.
inline bool pd_add_50(int64_t quot, char rem, __m512i *pd) {
    assert(quot < 50);
    // The header has size 50 + 51
    unsigned __int128 header = 0;
    // We need to copy (50+51) bits, but we copy slightly more and mask out the ones we
    // don't care about.
    //
    // memcpy is the only defined punning operation
    const unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;
    assert(kBytes2copy < sizeof(header));
    memcpy(&header, pd, kBytes2copy);
    // Number of bits to keep. Requires little-endianness
    const unsigned __int128 kLeftover = sizeof(header) * CHAR_BIT - 50 - 51;
    const unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
    header = header & kLeftoverMask;
    assert(popcount128(header) == 50);
    const unsigned fill = select128(header, 50 - 1) - (50 - 1);
    if (fill == 51) return false;
    // [begin,end) are the zeros in the header that correspond to the fingerprints with
    // quotient quot.
    const uint64_t begin = quot ? (select128(header, quot - 1) + 1) : 0;
    const uint64_t end = select128(header, quot);
    assert(begin <= end);
    assert(end <= 50 + 51);
    unsigned __int128 new_header = header & ((((unsigned __int128) 1) << begin) - 1);
    new_header |= ((header >> end) << (end + 1));
    assert(popcount128(new_header) == 50);
    assert(select128(new_header, 50 - 1) - (50 - 1) == fill + 1);
    memcpy(pd, &new_header, kBytes2copy);
    const uint64_t begin_fingerprint = begin - quot;
    const uint64_t end_fingerprint = end - quot;
    assert (begin_fingerprint <= end_fingerprint);
    assert(end_fingerprint <= 51);
    uint64_t i = begin_fingerprint;
    for (; i < end_fingerprint; ++i) {
        if (rem <= ((const char *) pd)[kBytes2copy + i]) break;
    }
    assert((i == end_fingerprint) || (rem <= ((const char *) pd)[kBytes2copy + i]));
    memmove(&((char *) pd)[kBytes2copy + i + 1], &((const char *) pd)[kBytes2copy + i],
            sizeof(*pd) - (kBytes2copy + 1 + 1) + 1);
    ((char *) pd)[kBytes2copy + i] = rem;
    assert(pd_find_50(quot, rem, pd));
    return true;
    // //// jbapple: STOPPED HERE
    // const __m512i target = _mm512_set1_epi8(rem);
    // uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd);
    // // round up to remove the header
    // v = v >> ((51 + 50 + CHAR_BIT - 1) / CHAR_BIT);
    // return (v & ((UINT64_C(1) << end) - 1)) >> begin;
}
