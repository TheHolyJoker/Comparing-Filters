//
// Created by tomer on 4/10/20.
//

#ifndef CLION_CODE_BIT_WORD_CONVERTER_HPP
#define CLION_CODE_BIT_WORD_CONVERTER_HPP

#include <iostream>
#include <cassert>
#include <vector>
#include <algorithm>
#include <ctgmath>
#include <climits>
#include <iomanip>
#include "../Global_functions/macros.h"
#include "my_bit_op.hpp"

template<typename T>
void print_vector(const vector<T> *v);

template<typename T>
void print_array_as_integers(const T *a, std::size_t size);

template<typename T>
auto print_array_as_consecutive_memory(const T *a, std::size_t size, std::ostream &os) -> std::ostream &;

template<typename T>
auto compare_vector_and_array(const vector<bool> *v, const T *a, size_t a_size) -> int;


template<typename T>
auto equality_vec_array(const vector<bool> *v, const T *a, size_t a_size, size_t bits_lim) -> int;

//template<typename T>
//auto equality_vec_array(const vector<bool> *v, const T *a, size_t a_size, size_t abs_a_bit_start, size_t abs_a_bit_end,
//                        size_t abs_v_bit_start, size_t abs_v_bit_end) -> int;


template<typename T>
void from_array_of_words_to_bit_vector(vector<bool> *vec, const T *a, size_t a_size);

template<typename T>
void from_bit_vector_to_array_of_words(const vector<bool> *v, T *a, size_t a_size);


/**
 * \print_bit_vector_as_words and \print_vector_by_unpacking are the same function.
 * @param v
 * @param word_size
 */
void print_bit_vector_as_words(const vector<bool> *v, size_t word_size = 32);

//template<typename T>
//void print_T_vector_as_words(const vector<T> *v, size_t word_size);

void print_vector_by_unpacking(const vector<bool> *v, size_t unpack_size);

void print_bool_vector_no_spaces(const vector<bool> *v);

template<typename T>
auto read_T_word_from_vector(const vector<bool> *v, size_t start, size_t end) -> T;


template<typename T>
void write_val_of_length_k_to_bit_vector(vector<bool> *v, T val, size_t abs_bit_start_index, size_t val_length);

/**
 *
 * Example takes a vector \val_vec containing \T, trim the leading bits in each value to get only \word_size length values.
 * Then stores it in a \dest. as bit vec.
 * @tparam T
 * @param val_vec
 * @param dest
 * @param word_size
 */
template<typename T>
void from_val_vector_to_bit_vector_with_given_word_size(const vector<T> *val_vec, vector<bool> *dest, size_t word_size);

/**
 * Each value x is replaced with "1" * x + "0". Result is stored in dest.
 * @tparam T
 * @param val_vec
 * @param dest
 */
template<typename T>
void from_val_vector_to_bit_vector_representing_PD_header(const vector<T> *val_vec, vector<bool> *dest);

/**
 *
 * @tparam T
 * @param arr
 * @param abs_bit_size size of header in bits.
 * @param max_capacity number of zeros in header
 * @param dest
 */
template<typename T>
void
from_val_vector_to_bit_vector_representing_PD_header_inverse(const T *arr, size_t abs_bit_size, size_t max_capacity,
                                                             vector<T> *dest);


auto read_FP_from_vector_by_index(const vector<bool> *v, size_t bit_start_index, size_t fp_size) -> uint32_t;

void write_FP_to_vector_by_index(vector<bool> *v, size_t index, uint32_t remainder, size_t fp_size);


/**
 *
 * @tparam T
 * @param vec
 * @param a
 * @param abs_bit_start_index inclusive
 * @param abs_bit_end_index NON-inclusive
 */
template<typename T>
void
from_array_to_vector_by_bit_limits(vector<bool> *vec, const T *a, size_t abs_bit_start_index, size_t abs_bit_end_index);


auto
from_array_to_vector_by_bit_limits_mid(size_t abs_bit_start_index, size_t abs_bit_end_index,
                                       size_t slot_size = 32u) -> std::tuple<size_t, size_t>;

template<typename T>
auto my_bin(T n, size_t length = 32) -> std::string;


void vector_differences_printer(const vector<bool> *valid_vec, const vector<bool> *att_vec);

void vector_differences_printer(const vector<bool> *valid_vec, const vector<bool> *att_vec, size_t word_size);

/////


template<typename T>
void store_header(vector<T> *vec);

template<typename T>
void store_remainders(vector<T> *vec);

template<typename T>
void store_counters(vector<T> *vec);


//// Don't use:
auto compare_vector_and_array(const vector<bool> *v, const uint32_t *a) -> int;

void to_vector(vector<bool> *vec, const uint32_t *a, size_t a_size);

void vector_to_word_array(const vector<bool> *v, uint32_t *a, size_t a_size);


#endif //CLION_CODE_BIT_WORD_CONVERTER_HPP
