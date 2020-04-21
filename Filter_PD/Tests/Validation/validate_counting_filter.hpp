//
// Created by tomer on 4/11/20.
//

#ifndef CLION_CODE_VALIDATE_COUNTING_FILTER_HPP
#define CLION_CODE_VALIDATE_COUNTING_FILTER_HPP

#include "validate_filter.hpp"
#include "../../Filters/Dict/multi_dict.hpp"

#define MAX_PART 42

template<class D>
auto
v_counting_filter_init(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv, size_t l1_counter_size,
                       size_t l2_counter_size, double level1_load_factor, double level2_load_factor) -> bool;

template<class D>
auto v_CF_wrapper(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv, size_t l1_counter_size,
                       size_t l2_counter_size, double level1_load_factor, double level2_load_factor) -> bool;

template<class D>
auto v_CF_wrapper(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv, size_t l1_counter_size,
                  size_t l2_counter_size, double level1_load_factor, double level2_load_factor, size_t seq_reps) -> bool;


/**
 *
 * @tparam D
 * @param filter
 * @param filter_max_capacity
 * @param lookup_reps
 * @param part how many stages to preform.
 * @return
 */
template<class D>
auto v_filter_no_deletions(D *filter, size_t filter_max_capacity, size_t lookup_reps, size_t part) -> bool;


template<class D>
auto v_CF_seq_op(D *filter, size_t new_elements_to_add ,size_t lookup_reps) ->bool;

template<class D>
auto v_CF_insert_single(D *filter, const string *s) -> bool;

template<class D>
auto v_CF_insert_overflow_single(D *filter) -> bool;


template<class D>
auto v_single_deletion(D *filter, const string *s) -> bool;

#endif //CLION_CODE_VALIDATE_COUNTING_FILTER_HPP
