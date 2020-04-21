//
// Created by tomer on 4/18/20.
//

#ifndef FILTER_COMPARISON_BENCHMARKING_FILTERS_HPP
#define FILTER_COMPARISON_BENCHMARKING_FILTERS_HPP



//#include "../Filter_PD/Tests/Benchmarking/benchmark_tests.h"
//#include "../Filter_PD/Tests/Benchmarking/benchmark_counting_filter.hpp"
#include "../Filter_PD/Tests/Test_output_formatting/test_printer.hpp"
#include "b_helpers.hpp"
#include "../Filter_BF/New_BF/General_BF/GeneralBF.h"
#include "../Filter_Quotient/QuotientFilter/Filter_QF.h"
#include "../Filter_PD/Filters/Dict/multi_dict.hpp"
#include "../Filter_PD/Filters/Dict/dict.hpp"

#include <chrono>

namespace name_compare {
    template<class D>
    auto
    b_filter_wrapper(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv, size_t l1_counter_size,
                     size_t l2_counter_size, double level1_load_factor, double level2_load_factor,
                     ostream &os) -> ostream &;

    template<class D>
    auto
    b_filter_rates_core(D *filter, size_t filter_max_capacity, size_t lookup_reps, ulong init_time, size_t error_power_inv,
                        std::ostream &os) -> std::ostream &;

    template<class D>
    auto b_time_lookups(D *filter, std::set<std::string> *element_set) -> ulong;

    template<class D>
    auto b_time_insertions(D *filter, std::set<std::string> *element_set) -> ulong;

    template<class D>
    auto b_time_deletions(D *filter, std::set<std::string> *element_set) -> ulong;

    template<class D>
    auto count_false_positives(D *filter, std::set<std::string> *lookup_set,
                               std::vector<std::set<std::string> *> *member_vec) -> std::size_t;
}

#endif //FILTER_COMPARISON_BENCHMARKING_FILTERS_HPP
