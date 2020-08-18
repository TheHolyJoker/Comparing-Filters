/*
 * Taken from this repository.
 * https://github.com/jbapple/crate-dictionary
 * */

#ifndef FILTERS_PD512_HPP
#define FILTERS_PD512_HPP

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>

#include <iostream>

// #include "immintrin.h"
#include <immintrin.h>
//#include "x86intrin.h"

namespace pd512 {
    // returns the position (starting from 0) of the jth set bit of x.
    inline uint64_t select64(uint64_t x, int64_t j) {
        assert(j < 64);
        const uint64_t y = _pdep_u64(UINT64_C(1) << j, x);
        return _tzcnt_u64(y);
    }

    // returns the position (starting from 0) of the jth set bit of x.
    inline uint64_t select128(unsigned __int128 x, int64_t j) {
        const int64_t pop = _mm_popcnt_u64(x);
        if (j < pop)
            return select64(x, j);
        return 64 + select64(x >> 64, j - pop);
    }

    inline uint64_t select128withPop64(unsigned __int128 x, int64_t j, int64_t pop) {
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

    inline int popcount64(uint64_t x) {
        return _mm_popcnt_u64(x);
    }

    inline int popcount128(unsigned __int128 x) {
        const uint64_t hi = x >> 64;
        const uint64_t lo = x;
        return popcount64(lo) + popcount64(hi);
    }

    inline bool pd_full(const __m512i *pd) {
        uint64_t header_end;
        memcpy(&header_end, reinterpret_cast<const uint64_t *>(pd) + 1,
               sizeof(header_end));
        return 1 & (header_end >> (50 + 51 - 64 - 1));
    }

    inline int pd_popcount(const __m512i *pd) {
        uint64_t header_end;
        memcpy(&header_end, reinterpret_cast<const uint64_t *>(pd) + 1,
               sizeof(header_end));
        constexpr uint64_t kLeftoverMask = (UINT64_C(1) << (50 + 51 - 64)) - 1;
        header_end = header_end & kLeftoverMask;
        const int result = 128 - 51 - _lzcnt_u64(header_end) + 1;
        return result;
    }


    // find an 8-bit value in a pocket dictionary with quotients in [0,64) and 49 values
    bool pd_find_64(int64_t quot, char rem, const __m512i *pd);

    inline bool pd_find_50(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        unsigned __int128 header = 0;
        memcpy(&header, pd, sizeof(header));
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        header = header & kLeftoverMask;
        // [begin,end) are the zeros in the header that correspond to the fingerprints
        // with quotient quot.
        const int64_t pop = _mm_popcnt_u64(header);
        const uint64_t begin = (quot ? (select128withPop64(header, quot - 1, pop) + 1) : 0) - quot;
        const uint64_t end = select128withPop64(header, quot, pop) - quot;
        assert(begin <= end);
        assert(end <= 51);
        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd);
        // round up to remove the header
        constexpr unsigned kHeaderBytes = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;
        assert(kHeaderBytes < sizeof(header));
        v = v >> kHeaderBytes;
        return (v & ((UINT64_C(1) << end) - 1)) >> begin;
    }


    // insert a pair of a quotient (mod 50) and an 8-bit remainder in a pocket dictionary.
    // Returns false if the dictionary is full.
    inline bool pd_add_50_old(int64_t quot, char rem, __m512i *pd) {
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
        assert((fill <= 14) || (fill == pd_popcount(pd)));
        assert((fill == 51) == pd_full(pd));
        if (fill == 51)
            return false;
        // [begin,end) are the zeros in the header that correspond to the fingerprints with
        // quotient quot.
        const uint64_t begin = quot ? (select128(header, quot - 1) + 1) : 0;
        const uint64_t end = select128(header, quot);
        assert(begin <= end);
        // assert(end <= 50 + 51);
        unsigned __int128 new_header = header & ((((unsigned __int128) 1) << begin) - 1);
        new_header |= ((header >> end) << (end + 1));
        assert(popcount128(new_header) == 50);
        assert(select128(new_header, 50 - 1) - (50 - 1) == fill + 1);
        memcpy(pd, &new_header, kBytes2copy);
        const uint64_t begin_fingerprint = begin - quot;
        const uint64_t end_fingerprint = end - quot;
        assert(begin_fingerprint <= end_fingerprint);
        assert(end_fingerprint <= 51);
        uint64_t i = begin_fingerprint;
        for (; i < end_fingerprint; ++i) {
            if (rem <= ((const char *) pd)[kBytes2copy + i])
                break;
        }
        assert((i == end_fingerprint) || (rem <= ((const char *) pd)[kBytes2copy + i]));
        memmove(&((char *) pd)[kBytes2copy + i + 1],
                &((const char *) pd)[kBytes2copy + i],
                sizeof(*pd) - (kBytes2copy + i + 1));
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
        assert((fill <= 14) || (fill == pd_popcount(pd)));
        assert((fill == 51) == pd_full(pd));
        if (fill == 51)
            return false;

        // [begin,end) are the zeros in the header that correspond to the fingerprints with
        // quotient quot.
        const uint64_t begin = quot ? (select128(header, quot - 1) + 1) : 0;
        const uint64_t end = select128(header, quot);
        assert(begin <= end);
        assert(end <= 50 + 51);
        const __m512i target = _mm512_set1_epi8(rem);


        unsigned __int128 new_header = header & ((((unsigned __int128) 1) << begin) - 1);
        new_header |= ((header >> end) << (end + 1));
        assert(popcount128(new_header) == 50);
        assert(select128(new_header, 50 - 1) - (50 - 1) == fill + 1);
        memcpy(pd, &new_header, kBytes2copy);

        const uint64_t begin_fingerprint = begin - quot;
        const uint64_t end_fingerprint = end - quot;
        assert(begin_fingerprint <= end_fingerprint);
        assert(end_fingerprint <= 51);

        // const uint64_t before_shift = _mm512_cmp_epi8_mask(target, *pd, 2);
        // uint64_t i = _tzcnt_u64(((before_shift >> (begin_fingerprint + 13)) << (begin_fingerprint)) | (1ull << end_fingerprint));
        // uint64_t i = _tzcnt_u64(((_mm512_cmp_epi8_mask(target, *pd, 2) >> (begin_fingerprint + 13)) << (begin_fingerprint)) | (1ull << end_fingerprint));
        // uint64_t i = _tzcnt_u64(((before_shift >> (begin_fingerprint + 13)) << (begin_fingerprint)) | (1ull << end_fingerprint));
        // uint64_t i = (begin == end) ? end_fingerprint : _tzcnt_u64(((before_shift >> (begin_fingerprint + 13)) << (begin_fingerprint)) | (1ull << end_fingerprint));
        uint64_t i = (begin == end) ? end_fingerprint : _tzcnt_u64(((_mm512_cmp_epi8_mask(target, *pd, 2) >> (begin_fingerprint + 13)) << (begin_fingerprint)) | (1ull << end_fingerprint));

        assert((i == end_fingerprint) || (rem <= ((const char *) pd)[kBytes2copy + i]));

        memmove(&((char *) pd)[kBytes2copy + i + 1],
                &((const char *) pd)[kBytes2copy + i],
                sizeof(*pd) - (kBytes2copy + i + 1));
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


    auto remove_naive(int64_t quot, char rem, __m512i *pd) -> bool;

    inline auto remove(int64_t quot, char rem, __m512i *pd) -> bool {
        assert(quot < 50);
        assert(pd_find_50(quot, rem, pd));
        // The header has size 50 + 51
        unsigned __int128 header = 0;
        // We need to copy (50+51) bits, but we copy slightly more and mask out the ones we
        // don't care about.
        //
        // memcpy is the only defined punning operation
        const unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;
        assert(kBytes2copy < sizeof(header));
        memcpy(&header, pd, kBytes2copy);
        // auto my_temp = popcount128(header);

        // std::cout << "my_temp: " << my_temp << std::endl;
        // Number of bits to keep. Requires little-endianness
        const unsigned __int128 kLeftover = sizeof(header) * CHAR_BIT - 50 - 51;
        const unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        header = header & kLeftoverMask;
        assert(popcount128(header) == 50);

        //// const unsigned fill = select128(header, 50 - 1) - (50 - 1);
        //// assert((fill <= 14) || (fill == pd_popcount(pd)));
        //// assert((fill == 51) == pd_full(pd));
        //// if (fill == 51)
        ////    return false;

        // [begin,end) are the zeros in the header that correspond to the fingerprints with
        // quotient quot.
        const uint64_t begin = quot ? (select128(header, quot - 1) + 1) : 0;
        const uint64_t end = select128(header, quot);
        assert(begin <= end);
        // assert(end <= 50 + 51);
        unsigned __int128 new_header = header & ((((unsigned __int128) 1) << begin) - 1);
        new_header |= header >> (end - 1);
        assert(popcount128(new_header) == 50);
        //// assert(select128(new_header, 50 - 1) - (50 - 1) == fill + 1);


        memcpy(pd, &new_header, kBytes2copy);
        const uint64_t begin_fingerprint = begin - quot;
        const uint64_t end_fingerprint = end - quot;
        assert(begin_fingerprint <= end_fingerprint);
        assert(end_fingerprint <= 51);
        uint64_t i = begin_fingerprint;
        for (; i < end_fingerprint; ++i) {
            if (rem == ((const char *) pd)[kBytes2copy + i])
                break;
        }
        assert(rem == ((const char *) pd)[kBytes2copy + i]);

        memmove(&((char *) pd)[kBytes2copy + i],
                &((const char *) pd)[kBytes2copy + i + 1],
                sizeof(*pd) - (kBytes2copy + i + 1));
        // ((char *) pd)[kBytes2copy + i] = rem;

        // assert(pd_find_50(quot, rem, pd));
        return true;
    }

    inline auto conditional_remove(int64_t quot, char rem, __m512i *pd) -> bool {
        assert(quot < 50);
        unsigned __int128 header = 0;
        const unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;
        assert(kBytes2copy < sizeof(header));
        memcpy(&header, pd, kBytes2copy);
        const unsigned __int128 kLeftover = sizeof(header) * CHAR_BIT - 50 - 51;
        const unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        header = header & kLeftoverMask;
        assert(popcount128(header) == 50);


        // [begin,end) are the zeros in the header that correspond to the fingerprints with
        // quotient quot.
        /* const int64_t pop = _mm_popcnt_u64(header);
        const uint64_t begin = (quot ? (select128withPop64(header, quot - 1, pop) + 1) : 0) - quot;
        const uint64_t end = select128withPop64(header, quot, pop) - quot;
        assert(begin <= end);
        assert(end <= 51); */

        const uint64_t begin = quot ? (select128(header, quot - 1) + 1) : 0;
        const uint64_t end = select128(header, quot);
        assert(begin <= end);

        const uint64_t begin_fingerprint = begin - quot;
        const uint64_t end_fingerprint = end - quot;
        assert(begin_fingerprint <= end_fingerprint);
        assert(end_fingerprint <= 51);
        uint64_t i = begin_fingerprint;
        // Consider using this function instead of this for.
        // __mmask64 _mm512_cmp_epu8_mask (__m512i a, __m512i b, const int imm8)

        for (; i < end_fingerprint; ++i) {
            if (rem <= ((const char *) pd)[kBytes2copy + i])
                break;
        }

        if ((i == end_fingerprint) || (rem != ((const char *) pd)[kBytes2copy + i])) {
            // std::cout << "pd512::CR" << std::endl;
            return false;
        }

        assert(pd_find_50(quot, rem, pd));
        unsigned __int128 new_header = header & ((((unsigned __int128) 1) << begin) - 1);
        new_header |= ((header >> end) << (end - 1));
        // new_header |= header >> (end - 1);
        assert(popcount128(new_header) == 50);

        memcpy(pd, &new_header, kBytes2copy);

        memmove(&((char *) pd)[kBytes2copy + i],
                &((const char *) pd)[kBytes2copy + i + 1],
                sizeof(*pd) - (kBytes2copy + i + 1));
        return true;
    }
    // inline auto conditional_remove(int64_t quot, char rem, __m512i *pd) -> bool {
    //     if (pd_find_50(quot % 51, rem, pd)) {
    //         remove(quot % 51, rem, pd);
    //         return true;
    //     }
    //     return false;
    // }

    void print512(const __m512i *var);


    inline auto pd_popcount_att_helper_start(const __m512i *pd) -> int {
        uint64_t header;
        memcpy(&header, reinterpret_cast<const uint64_t *>(pd), 8);
        return 14ul - _lzcnt_u64(header);
    }

    inline auto get_capacity(const __m512i *pd) -> int {
        uint64_t header_end;
        memcpy(&header_end, reinterpret_cast<const uint64_t *>(pd) + 1, 5);
        constexpr uint64_t mask = (1ULL << 37u) - 1u;
        header_end &= mask;
        return (header_end == 0) ? pd_popcount_att_helper_start(pd) : (128 - 51 - _lzcnt_u64(header_end) + 1);
    }


    auto validate_number_of_quotient(const __m512i *pd) -> bool;

    auto get_capacity_old(const __m512i *x) -> size_t;

    auto get_capacity_naive(const __m512i *x) -> size_t;

    auto get_name() -> std::string;
    ////New functions
    inline auto is_full(const __m512i *x) -> bool {
        assert(get_capacity_naive(x) == get_capacity(x));
        return get_capacity(x) == 51;
    }
}// namespace pd512


namespace v_pd512 {


}// namespace v_pd512

#endif// FILTERS_PD512_HPP


auto my_equal(__m512i x, __m512i y) -> bool;
