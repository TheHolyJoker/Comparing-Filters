//
// Created by tomer on 4/21/20.
//

#ifndef FILTER_COMPARISON_BENCHMARK_INTEGER_HPP
#define FILTER_COMPARISON_BENCHMARK_INTEGER_HPP

#include "../Filter_PD/Tests/Test_output_formatting/test_printer.hpp"
#include "b_helpers.hpp"
#include "../Filter_BF/New_BF/General_BF/GeneralBF.h"
#include "../Filter_Quotient/QuotientFilter/Filter_QF.h"
#include "../Filter_PD/Filters/Dict/multi_dict.hpp"
#include "../Filter_PD/Filters/Dict/dict.hpp"
#include "benchmark_integer.hpp"
#include <cuckoofilter/cuckoofilter.h>

#define CUCKOO_BITS_PER_ELEMENTS 4

#include <chrono>

#define INT_TYPE uint32_t

//typdef <cuckoofilter::CuckooFilter,uint32_t> basic_cf<uinuint32_t ,CUCKOO_BITS_PER_ELEMENTS>;
//typdef <cuckoofilter::CuckooFilter,uint32_t> <uint32_t, CUCKOO_BITS_PER_ELEMENTS> basic_cf;
typedef cuckoofilter::CuckooFilter<uint32_t, CUCKOO_BITS_PER_ELEMENTS> basic_cf;
//typdef

namespace name_compare {
    template<class D, typename T>
    auto
    b_filter_wrapper_int(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv, size_t l1_counter_size,
                         size_t l2_counter_size, double level1_load_factor, double level2_load_factor,
                         ostream &os) -> ostream &;

    template<class D, typename T>
    auto
    b_filter_rates_core_int(D *filter, size_t filter_max_capacity, size_t lookup_reps, ulong init_time,
                            size_t error_power_inv,
                            std::ostream &os) -> std::ostream &;

    template<class D, typename T>
    auto b_time_lookups_int(D *filter, std::set<T> *element_set) -> ulong;

    template<class D, typename T>
    auto b_time_insertions_int(D *filter, std::set<T> *element_set) -> ulong;

    template<class D, typename T>
    auto b_time_deletions_int(D *filter, std::set<T> *element_set) -> ulong;

    template<class D, typename T>
    auto count_false_positives_int(D *filter, std::set<T> *lookup_set,
                                   std::vector<std::set<T> *> *member_vec) -> std::size_t;
}
#endif //FILTER_COMPARISON_BENCHMARK_INTEGER_HPP
