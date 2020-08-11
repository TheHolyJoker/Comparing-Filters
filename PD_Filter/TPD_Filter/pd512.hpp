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
    uint64_t select64(uint64_t x, int64_t j);

    // returns the position (starting from 0) of the jth set bit of x.
    uint64_t select128(unsigned __int128 x, int64_t j);

    // returns the number of zeros before the jth (counting from 0) set bit of x
    uint64_t nth64(uint64_t x, int64_t j);

    // returns the number of zeros before the jth (counting from 0) set bit of x
    uint64_t nth128(unsigned __int128 x, int64_t j);

    // returns the position (starting from 0) of the jth set bit of x. Takes as
    // input the popcnt of the low 64 bits of x.
    inline uint64_t select128withPop64(unsigned __int128 x, int64_t j, int64_t pop) {
        if (j < pop) return select64(x, j);
        return 64 + select64(x >> 64, j - pop);
    }

    int popcount64(uint64_t x);

    int popcount128(unsigned __int128 x);

    // find an 8-bit value in a pocket dictionary with quotients in [0,64) and 49
    // values
    bool pd_find_64(int64_t quot, char rem, const __m512i *pd);

    // find an 8-bit value in a pocket dictionary with quotients in [0,50) and 51
    // values
    bool pd_find_50_old(int64_t quot, char rem, const __m512i *pd);

    inline bool pd_find_50(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        unsigned __int128 header = 0;
        memcpy(&header, pd, sizeof(header));
        constexpr unsigned __int128 kLeftoverMask =
                (((unsigned __int128) 1) << (50 + 51)) - 1;
        header = header & kLeftoverMask;
        // [begin,end) are the zeros in the header that correspond to the fingerprints
        // with quotient quot.
        const int64_t pop = _mm_popcnt_u64(header);
        const uint64_t begin =
                (quot ? (select128withPop64(header, quot - 1, pop) + 1) : 0) - quot;
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


    // insert a pair of a quotient (mod 50) and an 8-bit remainder in a pocket
    // dictionary. Returns false if the dictionary is full.
    bool pd_add_50(int64_t quot, char rem, __m512i *pd);

    auto remove(int64_t quot, char rem, __m512i *pd) -> bool;

    auto conditional_remove(int64_t quot, char rem, __m512i *pd) -> bool;

    void print512(const __m512i *var);

    auto validate_number_of_quotient(const __m512i *pd) -> bool;

    auto is_full(const __m512i *x) -> bool;

    auto get_capacity(const __m512i *x) -> size_t;

    auto get_capacity_naive(const __m512i *x) -> size_t;

    auto get_name() -> std::string;

    template<class T>
    bool is_aligned(const void *ptr) noexcept {
        auto iptr = reinterpret_cast<std::uintptr_t>(ptr);
        return !(iptr % alignof(T));
    }

    ////New functions
    int pd_popcount(const __m512i *pd);
    bool pd_full(const __m512i *pd);

}// namespace pd512
#endif// FILTERS_PD512_HPP


auto my_equal(__m512i x, __m512i y) -> bool;
