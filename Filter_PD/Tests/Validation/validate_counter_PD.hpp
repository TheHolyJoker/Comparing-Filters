//
// Created by tomer on 4/10/20.
//

#ifndef CLION_CODE_VALIDATE_COUNTER_PD_HPP
#define CLION_CODE_VALIDATE_COUNTER_PD_HPP

#include "../../CPD/CPD_validator.hpp"
#include <random>
#include <set>


enum operation {
    op_insert, op_remove, op_lookup, op_multi_lookup
};

auto v_CPD(size_t reps, size_t max_capacity, size_t element_length, size_t counter_size, double working_load_factor,
           double variance, bool to_seed = false) -> bool;

auto op_sequence_iter(size_t reps, size_t max_capacity, size_t element_length, size_t counter_size,
                      double working_load_factor, double variance, bool to_seed = false) -> bool;
auto
v_wrapper(size_t max_capacity, size_t element_length, size_t counter_size, double working_load_factor,
                 bool to_seed = false) -> bool;


auto
v_wrapper_single(size_t max_capacity, size_t element_length, size_t counter_size, double working_load_factor,
                 bool to_seed = false) -> bool;

auto v_built_validator(CPD_validator *vd, size_t max_capacity, size_t element_length, size_t counter_size,
                       double working_load_factor, bool to_seed)->bool;

auto v_lookup_all_elements(CPD_validator *vd, vector<vector<q_r>> *el_vec) -> bool;

auto v_lookup_multi_all_elements(CPD_validator *vd, vector<vector<q_r>> *el_vec) -> bool;

auto v_dec_multiplicity(CPD_validator *vd, vector<vector<q_r>> *el_vec) -> bool;

auto v_remove_element(CPD_validator *vd, vector<vector<q_r>> *el_vec) -> bool;


auto v_lookup_helper(CPD_validator *vd, CPD_TYPE q, CPD_TYPE r, bool exp_res) -> bool;

auto v_lookup_multi_helper(CPD_validator *vd, CPD_TYPE q, CPD_TYPE r, size_t exp_res) -> bool;

void v_remove_helper(CPD_validator *vd, CPD_TYPE q, CPD_TYPE r, counter_status exp_res);

auto get_operation(CPD_validator &vd, double max_load_factor) -> operation;

auto do_op(CPD_validator &vd, operation op);

auto lookup(CPD_validator &vd, CPD_TYPE q, CPD_TYPE r) -> bool;

auto v_lookup_multi_wrapper(CPD_validator &vd, CPD_TYPE q, CPD_TYPE r) -> bool;

auto insert(CPD_validator &vd, CPD_TYPE q, CPD_TYPE r) -> bool;

auto remove(CPD_validator &vd, CPD_TYPE q, CPD_TYPE r) -> bool;

#endif //CLION_CODE_VALIDATE_COUNTER_PD_HPP
