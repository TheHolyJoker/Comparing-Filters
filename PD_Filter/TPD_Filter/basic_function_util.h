//
// Created by tomereven on 18/07/2020.
//

#ifndef FILTERS_BASIC_FUNCTION_UTIL_H
#define FILTERS_BASIC_FUNCTION_UTIL_H

#include <cstring>
#include "printutil.hpp"




template<typename T>
static auto ceil_log2(T x) -> size_t {
    assert(x > 1);
    size_t res = std::ceil(log2(x));
    assert ((1ULL << res) >= x);
    return res;
}

auto compute_number_of_PD(size_t max_number_of_elements, size_t max_capacity, double l1_load) -> size_t;

auto compute_spare_element_size(size_t max_number_of_elements, float level1_load_factor,
                                size_t pd_max_capacity = 51,size_t quot_range = 50,size_t rem_length = 8) -> size_t;


auto pd_filter_total_byte_size(size_t max_number_of_elements, size_t max_capacity, double l1_load, double l2_load) ->size_t;
#endif //FILTERS_BASIC_FUNCTION_UTIL_H


