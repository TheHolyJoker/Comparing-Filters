//
// Created by tomereven on 18/07/2020.
//

#ifndef FILTERS_BASIC_FUNCTION_UTIL_H
#define FILTERS_BASIC_FUNCTION_UTIL_H

#include "printutil.hpp"
#include <cstring>
#include <fstream>
#include <sstream>


template<typename T>
static auto ceil_log2(T x) -> size_t {
    assert(x > 1);
    size_t res = std::ceil(log2(x));
    assert((1ULL << res) >= x);
    return res;
}

auto compute_number_of_PD(size_t max_number_of_elements, size_t max_capacity, double l1_load, bool round_to_upperpower2 = false) -> size_t;

auto compute_spare_element_size(size_t max_number_of_elements, float level1_load_factor,
                                size_t pd_max_capacity = 51, size_t quot_range = 50, size_t rem_length = 8) -> size_t;

inline uint64_t upperpower2(uint64_t x) {
  x--;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  x |= x >> 32;
  x++;
  return x;
}



auto pd_filter_total_byte_size(size_t max_number_of_elements, size_t max_capacity, double l1_load, double l2_load) -> size_t;

/* Taken from the Xor filter repository https://github.com/FastFilter/fastfilter_cpp.*/

__attribute__((always_inline))
inline uint16_t reduce16(uint16_t hash, uint16_t n) {
    // http://lemire.me/blog/2016/06/27/a-fast-alternative-to-the-modulo-reduction/
    return (uint16_t) (((uint32_t) hash * n) >> 16);
}

__attribute__((always_inline))
inline uint32_t reduce32(uint32_t hash, uint32_t n) {
    // http://lemire.me/blog/2016/06/27/a-fast-alternative-to-the-modulo-reduction/
    return (uint32_t) (((uint64_t) hash * n) >> 32);
}

__attribute__((always_inline))
inline auto reduce64(uint64_t x, uint64_t mod) -> uint64_t {
    return (uint64_t)(((__uint128_t) x * (__uint128_t) mod) >> 64);
}

#endif//FILTERS_BASIC_FUNCTION_UTIL_H
