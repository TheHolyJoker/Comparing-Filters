//
// Created by tomer on 10/25/19.
//

#ifndef CLION_CODE_VALIDATION_TESTS_H
#define CLION_CODE_VALIDATION_TESTS_H


#include <tuple>
#include <random>

#include "../../PD/Non_Contiguous_PD/PD.h"
#include "../../PD/Safe_PD/safe_PD.h"
#include "../../PD/Naive_PD/naive_PD.h"
#include "../../PD/Constant_size_PD/const_PD.h"
#include "../../PD/Contiguous_PD/cg_PD.h"
#include "../../CPD/CPD.hpp"
#include "../../Filters/POW2C/pow2c_filter.h"
#include "../../Filters/POW2C/pow2c_naive_filter.h"
#include "../Benchmarking/benchmark_tests.h"

namespace namePD {
    void break_point_helper();

    auto validate_header_get_interval_function(size_t reps) -> bool;

    auto validate_get_interval_function_constant(size_t reps) -> bool;

    auto validate_push_function(size_t reps) -> bool;

    auto validate_pull_function(size_t reps) -> bool;

    auto validate_rank_function(size_t reps) -> bool;

/**
 * smashing stack for some reason.
 * @param reps
 * @return
 */
    auto validate_header_type(size_t reps) -> bool;

    auto t1(bool to_print = false) -> bool;

    auto t2(bool to_print = false) -> bool;

    auto t3(bool to_print = false) -> bool;

    auto t4(bool to_print = false) -> bool;

    auto t5(size_t m, size_t f, size_t l, bool to_print = false) -> bool;


    template<class T>
    auto validate_CPD(size_t reps, bool to_seed, bool to_print = 0) -> bool;


    template<class T>
    auto validate_PD(size_t reps, bool to_seed, bool to_print = 0) -> bool;

    template<class T>
    auto validate_PD_higher_load(size_t reps, float load_factor, bool to_seed, bool to_print = 0) -> bool;

/**
 * random tests.
 * @param reps
 * @param to_print
 * @return
 */
    auto validate_PD_single_run(size_t reps, bool to_print = 0) -> bool;

    auto validate_const_PD(size_t reps, bool to_print = 0) -> bool;

    template<class T>
    auto validate_PD_core(size_t reps, T *d, size_t m, size_t f, size_t l, bool to_print, float load_factor) -> bool;

    auto r0_core(size_t reps, PD *d, size_t m, size_t f, size_t l, bool to_print = 0) -> bool;


    auto
    validate_PD_by_load_factor(size_t reps, float load_factor, size_t m = 32, size_t f = 32, bool to_print = 0) -> bool;

    auto validate_safe_PD(size_t reps, bool to_print = 0) -> bool;

    auto validate_safe_PD_const_case(size_t reps, bool to_print = 0) -> bool;

    auto naive_pd_r0(size_t reps, bool to_print = 0) -> bool;

    auto r1(size_t reps, bool to_print) -> bool;

    auto vector_rw_t1() -> bool;

    auto pow2c_r0(size_t reps, bool to_print = 0) -> bool;

    auto pow2c_naive_r0(size_t reps, bool to_print = 0) -> bool;


//bool naive_pd_r0(size_t reps, bool to_print = 0) ->bool;
}
#endif //CLION_CODE_VALIDATION_TESTS_H
