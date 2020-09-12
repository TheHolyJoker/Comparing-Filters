//
// Created by root on 31/05/2020.
//

#include "new_tests.hpp"

auto example1() {
    ulong shift = 16u;
    size_t filter_max_capacity = 1u << shift;
    size_t lookup_reps = 1u << (shift + 2u);
    size_t error_power_inv = BITS_PER_ELEMENT_MACRO;
    size_t bench_precision = 16;
    // b_all_wrapper<uint64_t, BITS_PER_ELEMENT_MACRO>(filter_max_capacity, lookup_reps, error_power_inv, bench_precision,
    //                                                 1, 1, 1, 1, 1);
}






