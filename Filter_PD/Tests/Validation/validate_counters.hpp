//
// Created by tomer on 3/17/20.
//

#ifndef CLION_CODE_VALIDATE_COUNTERS_HPP
#define CLION_CODE_VALIDATE_COUNTERS_HPP

#include "validate_bit_op.hpp"
#include "../../CPD/Counters_Array.hpp"
#include "../../bit_operations/my_bit_op.hpp"

/**
 *
 * @tparam T
 * @param a array.
 * @param a_size The size of the array.
 * @param bit_start_index Index of the next bit after the delimiter.
 * @param bit_end_index Index of the first bit of the first delimiter after "bit_start_index".
 *
 *
 *                      (1) points to 1.
 *                      (2) bit_start_index <= bit_end_index
 *                      (3) a[bit_start_index:bit_end_index) contains no 1's.
 */
template<typename T>
auto v_extract_symbol_single(T *a, size_t a_size, size_t bit_start_index, size_t bit_end_index) -> bool;

template<typename T>
auto v_extract_symbol_iter(size_t reps, size_t a_size) -> bool;

template<typename T>
auto v_extract_symbol_rand(size_t reps, size_t a_size) -> bool;

auto v_get_symbols_length_difference(size_t reps) -> bool;

#endif //CLION_CODE_VALIDATE_COUNTERS_HPP
