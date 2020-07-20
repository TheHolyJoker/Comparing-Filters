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

#include "immintrin.h"
//#include "x86intrin.h"

namespace pd512
{
    // returns the position (starting from 0) of the jth set bit of x.
    uint64_t select64(uint64_t x, int64_t j);

    // returns the position (starting from 0) of the jth set bit of x.
    uint64_t select128(unsigned __int128 x, int64_t j);

    // returns the number of zeros before the jth (counting from 0) set bit of x
    uint64_t nth64(uint64_t x, int64_t j);

    // returns the number of zeros before the jth (counting from 0) set bit of x
    uint64_t nth128(unsigned __int128 x, int64_t j);

    int popcount64(uint64_t x);

    int popcount128(unsigned __int128 x);

    // find an 8-bit value in a pocket dictionary with quotients in [0,64) and 49 values
    bool pd_find_64(int64_t quot, char rem, const __m512i *pd);

    // find an 8-bit value in a pocket dictionary with quotients in [0,50) and 51 values
    bool pd_find_50(int64_t quot, char rem, const __m512i *pd);

    // insert a pair of a quotient (mod 50) and an 8-bit remainder in a pocket dictionary.
    // Returns false if the dictionary is full.
    bool pd_add_50(int64_t quot, char rem, __m512i *pd);

    void print512(__m512i var);

    template <class T>
    bool
    is_aligned(const void *ptr) noexcept
    {
        auto iptr = reinterpret_cast<std::uintptr_t>(ptr);
        return !(iptr % alignof(T));
    }

} // namespace pd512
#endif // FILTERS_PD512_HPP
