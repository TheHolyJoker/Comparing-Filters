//
// Created by tomer on 4/24/20.
//

#ifndef FILTER_COMPARISON_BENCHMARK_USING_MULTISETS_HPP
#define FILTER_COMPARISON_BENCHMARK_USING_MULTISETS_HPP

#include "benchmark_integer.hpp"


namespace name_compare {

    enum ops {
        insert, lookup, remove, count_FP
    };

    template<class filter_type, typename block_type>
    auto
    b_filter_wrapper_multi(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv,
                           size_t l1_counter_size,
                           size_t l2_counter_size, double level1_load_factor, double level2_load_factor,
                           size_t universe_size,
                           ostream &os) -> ostream &;

    template<class filter_type, typename block_type>
    auto
    b_filter_rates_core_multi(filter_type *filter, size_t filter_max_capacity, size_t lookup_reps, ulong init_time,
                              size_t error_power_inv, size_t universe_size,
                              std::ostream &os) -> std::ostream &;

    template<class filter_type, typename block_type, class set_type>
    auto do_single_op(filter_type *filter, set_type *element_set, ops op) -> ulong;

    template<class filter_type, typename block_type, class set_type>
    auto
    do_single_op(filter_type *filter, set_type *lookup_set, std::vector<set_type *> *member_vec, ops op) -> std::size_t;
/*
    template<class D, typename T, typename set_type>
    auto b_time_lookups_int(D *filter, set_type<T> *element_set) -> ulong;

    template<class D, typename T, typename set_type>
    auto b_time_insertions_int(D *filter, set_type<T> *element_set) -> ulong;

    template<class D, typename T, typename set_type>
    auto b_time_deletions_int(D *filter, set_type<T> *element_set) -> ulong;

    template<class D, typename T, typename set_type>
    auto count_false_positives_int(D *filter, set_type<T> *lookup_set,
                                   std::vector<set_type<T> *> *member_vec) -> std::size_t;*/
}

#endif //FILTER_COMPARISON_BENCHMARK_USING_MULTISETS_HPP
