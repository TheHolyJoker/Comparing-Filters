//
// Created by tomer on 4/9/20.
//

#ifndef CLION_CODE_CPD_VALIDATOR_HPP
#define CLION_CODE_CPD_VALIDATOR_HPP

#include <ostream>
#include "CPD.hpp"

typedef std::tuple<CPD_TYPE, CPD_TYPE> q_r;
typedef vector<q_r> v_q_r;
typedef vector<v_q_r> vv_q_r;

class CPD_validator {
    CPD d;
    vector<CPD_TYPE> H_vec, B_vec, C_vec;
    const size_t max_distinct_capacity, remainder_length, counter_size, quotient_range;

public:

    CPD_validator(size_t quotient_range, size_t max_distinct_capacity, size_t remainder_length, size_t counter_size);

    CPD_validator(size_t max_distinct_capacity, size_t remainder_length, size_t counter_size);

    auto lookup(CPD_TYPE q, CPD_TYPE r) -> bool;

    auto lookup_multi(CPD_TYPE q, CPD_TYPE r) -> size_t;

    auto insert(CPD_TYPE q, CPD_TYPE r) -> counter_status;

    auto insert_inc_attempt(CPD_TYPE q, CPD_TYPE r) -> counter_status;

    void remove(CPD_TYPE q, CPD_TYPE r);

    auto conditional_remove(CPD_TYPE q, CPD_TYPE r) -> counter_status;

    auto naive_comparison() -> bool;

    auto compare_all() -> bool;

    auto compare_header() -> bool;

    auto compare_body() -> bool;

    auto compare_counters() -> bool;

    auto is_empty() -> bool;

    auto is_full() -> bool;

    auto get_capacity() -> size_t;

    auto sum_header_values() -> size_t;

    void get_elements(vector<vector<q_r>> *el_vec);

    auto get_counter(size_t counter_index) -> CPD_TYPE;

    void set_counter(size_t counter_index, size_t new_val);

    void insert_new_element_with_counter(CPD_TYPE q, CPD_TYPE r,CPD_TYPE counter);

private:
    auto v_lookup(CPD_TYPE q, CPD_TYPE r) -> bool;

    auto v_lookup_multi(CPD_TYPE q, CPD_TYPE r) -> size_t;

    auto v_insert(CPD_TYPE q, CPD_TYPE r) -> counter_status;

    auto v_remove(CPD_TYPE q, CPD_TYPE r) -> counter_status;

    auto v_conditional_remove(CPD_TYPE q, CPD_TYPE r) -> counter_status;


    void H_find(CPD_TYPE q, size_t *start, size_t *end);

//    auto B_find(CPD_TYPE r, size_t unpacked_start_index, size_t unpacked_end_index, size_t *A_index,
//                size_t *rel_bit_index, size_t *r_unpack_index) -> bool;

    auto B_find(CPD_TYPE r, size_t unpacked_start_index, size_t unpacked_end_index, size_t *r_unpack_index) -> bool;

    void insert_new_element(CPD_TYPE q, CPD_TYPE r);

    auto H_insert(CPD_TYPE q, size_t *start, size_t *end);

    void
    B_insert_new_element(CPD_TYPE r, size_t unpacked_start_index, size_t unpacked_end_index, size_t *r_unpack_index);

    void remove_element_completely(CPD_TYPE q, size_t unpacked_index);

    auto increase_counter(size_t counter_index) -> counter_status;

    auto decrease_counter(size_t counter_index) -> counter_status;

    auto get_CPD_header_size_in_bits() -> size_t;

    auto get_CPD_body_size_in_bits() -> size_t;

    auto get_CPD_counters_size_in_bits() -> size_t;

    void prepare_B_vec_for_comparison(vector<CPD_TYPE> *v);

    counter_status v_insert_inc_attempt(uint32_t q, uint32_t r);
};


#endif //CLION_CODE_CPD_VALIDATOR_HPP
