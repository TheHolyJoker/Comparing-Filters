//
// Created by tomer on 3/15/20.
//

#ifndef CLION_CODE_MY_BIT_OP_HPP
#define CLION_CODE_MY_BIT_OP_HPP

#include <iostream>
#include <zconf.h>
#include <vector>
#include <cstdint>
#include <cassert>
#include <ostream>
#include "../Global_functions/macros.h"
#include "bit_op.h"
#include "../Global_functions/basic_functions.h"
//#include "bit_word_converter.hpp"


using namespace std;
typedef unsigned int u32;


/**
 *
 * @tparam T
 * @param x
 * @param y
 * @return length of y - length of x.
 */
template<typename T>
auto compute_diff_value_safe(T x, T y) -> int;

/**
 *
 * @tparam T
 * @param x
 * @param y
 * @return floor(log2(x)) - floor(log2(y)) (log2(0) = 0?)
 * //todo __builtin_clz(0) == 0?
 */
template<typename T>
auto get_numbers_bit_length_difference(const T x, const T y) -> int;

/**
 * Get the minimal number of bits one needs to store x;
 * @tparam T
 * @param x
 * @return
 */
template<typename T>
auto get_x_bit_length(const T x) -> size_t;

/**
 * Rank and Select operations definition: https://en.wikipedia.org/wiki/Succinct_data_structure#Succinct_dictionaries
 * @tparam T
 * @param a
 * @param a_size
 * @param k
 * @param start Stores result of SELECT(a, k) + 1
 * @param end Stores result of SELECT(a, k + 1)
 */
template<typename T>
void find_kth_interval_simple(T *a, size_t a_size, size_t k, size_t *start, size_t *end);

template<typename T>
auto find_kth_interval_simple(T *a, size_t a_size, size_t k) -> std::tuple<size_t, size_t>;

/**
 * Preforms SELECT, under the assumption "word" contains MORE than k set bits.
 * @param word
 * @param k
 * @param start
 * @param end
 */
void word_k_select(uint32_t word, size_t k, size_t *start, size_t *end);

/**
 *
 * @tparam T
 * @param a
 * @param a_size
 * @return The index of the set bit in "a" (were "a" is a vector of bits.)
 */
template<typename T>
auto find_first_set_bit(T *a, size_t a_size) -> size_t;

/**
 * Special case of "find_kth_interval_simple" where k is 0;
 * @tparam T
 * @param a
 * @param a_size
 * @param first
 * @param second
 */
template<typename T>
void find_first_and_second_set_bits(const T *a, size_t a_size, size_t *first, size_t *second);

template<typename T>
auto find_first_and_second_set_bits(const T *a, size_t a_size) -> std::tuple<size_t, size_t>;


/**
 * Count the number of sets bits. ? sum of "rank" on every element ?
 * @param a
 * @param a_size
 * @return
 */
auto count_set_bits(uint32_t *a, size_t a_size) -> size_t;

/**
 * Reads word from A.
 * @tparam T
 * @param A
 * @param a_size
 * @param bit_start_index
 * @param bit_end_index
 * @return
 * In python syntax, assuming "A" is a string:
 * return bin(A[bit_start_index:bit_end_index],2)
 *
 */
template<typename T>
auto read_word(const T *A, size_t a_size, size_t bit_start_index, size_t bit_end_index) -> T;

/**
 * Assume all elements in "A" have the same length.
 * @tparam T The array type
 * @param A The array
 * @param a_size maximal number of slots in the array (redundant)
 * @param first_element_index the index (in the abstract array) of the first element we read.
 * @param element_length every element is encoded in exactly this number of bits.
 * @param res_array where the elements should be stored
 * @return
 */
template<typename T>
void
read_k_words_fixed_length(const T *A, size_t a_size, size_t first_element_index, size_t element_length, T *res_array);

/**
 * Assume all elements in "A" have the same length.
 * @tparam T The array type
 * @param A The array
 * @param a_size number of slots in "A".
 * @param first_element_index the index (in the abstract array) of the first element we read.
 * @param element_length every element is encoded in exactly this number of bits.
 * @param res_array where the elements should be stored. its length matches the number of words we will read.
 * @param k The number of words we will read
 * @return
 */
template<typename T>
void
read_k_words_fixed_length_att(const T *A, size_t a_size, size_t first_element_index, size_t element_length,
                              T *res_array, size_t k);


/**
 * Write a word to A.
 * prev_start and prev_end are needed to know which bits to keep.
 * @tparam T
 * @param a
 * @param prev_start
 * @param prev_end
 * @param new_start
 * @param new_end
 * @param new_val
 * @param a_size
 */
template<typename T>
void update_element(T *a, size_t prev_start, size_t prev_end, size_t new_start, size_t new_end, T new_val,
                    size_t a_size);

/**
 * Deals with longer element
 * @tparam T
 * @param a
 * @param prev_start
 * @param prev_end
 * @param new_start
 * @param new_end
 * @param new_val
 * @param a_size
 */
template<typename T>
void update_element_push(T *a, size_t prev_start, size_t prev_end, size_t new_start, size_t new_end, T new_val,
                         size_t a_size);

/**
 * Deals with shorter element.
 * @tparam T
 * @param a
 * @param prev_start
 * @param prev_end
 * @param new_start
 * @param new_end
 * @param new_val
 * @param a_size
 */
template<typename T>
void update_element_pull(T *a, size_t prev_start, size_t prev_end, size_t new_start, size_t new_end, T new_val,
                         size_t a_size);

/**
 * Change the value between "start" and "end" to "new_val".
 * assertion error if new_val needs more than (end - start) bits.
 * @tparam T
 * @param a
 * @param bit_start_pos
 * @param bit_end_pos
 * @param a_size
 * @param new_val
 */
template<typename T>
void update_element_with_fixed_size(T *a, size_t bit_start_pos, size_t bit_end_pos, T new_val, size_t a_size);




////Functions used for validation.

auto vector_last_k_bits_are_zero(vector<bool> *vec, size_t k) -> bool;

/**
 * Reads a word as type T.
 * @tparam T
 * @param v vector of bits.
 * @param start beginning of thw word (included)
 * @param end first bit after the word.
 * @return
 * In python syntax: return bin(vec[start:end],2)
 * If type T does not have enough bits to store the word, the function will assert?.
 */
template<typename T>
auto read_T_word(const vector<bool> *v, size_t start, size_t end) -> T;

//template<typename T>
//void from_array_to_vector(vector<bool> *vec, const T *a, size_t a_size);

/*

template<typename T>
void from_vector_to_array(const vector<bool> *vec, T *a, size_t a_size);
*/


/**
 *
 * @tparam T
 * @param vec
 * @param a
 * @param a_size
 * @param word_size
 * In python syntax, where vec is binary string:
 *      assert a_size == (vec.size() / word_size)
 *      \*set in a*\: [int(vec[i:(i+1)*word_size,2) for i in range(a_size)]
 */
template<typename T>
void split_bit_vector_to_words_array(const vector<bool> *vec, T *a, size_t a_size, size_t word_size);

/**
 *
 * @tparam T
 * @param v
 * @param start
 * @param end
 * @return With python syntax, assuming v is a string of {0,1}*: return int(v[start:end],2)
 */
template<typename T>
auto sub_vector_to_word(const vector<bool> *v, size_t start, size_t end) -> T;


/**
 *
 * @param vec
 * @param k
 * @param start_index The index of the (k-1) set bit (-1)
 * @param end_index
 */
void vector_find_kth_interval_simple(vector<bool> *vec, size_t k, size_t *start_index, size_t *end_index);


auto vector_find_first_set_bit(vector<bool> *vec) -> size_t;

auto vector_extract_symbol(vector<bool> *vec, size_t *start_index, size_t *end_index) -> uint32_t;

template<typename T>
void vector_update_element_with_fixed_size(vector<bool> *vec, size_t start, size_t end, T new_val);

template<typename T>
void vector_update_element(vector<bool> *vec, size_t prev_start, size_t prev_end, size_t new_start, size_t new_end,
                           T new_val);

template<typename T>
void
vector_update_push(vector<bool> *vec, size_t prev_start, size_t prev_end, size_t new_start, size_t new_end, T new_val);


template<typename T>
void
vector_update_pull(vector<bool> *vec, size_t prev_start, size_t prev_end, size_t new_start, size_t new_end, T new_val);



//void vector_find_first_and_second_set_bits()

#endif //CLION_CODE_MY_BIT_OP_HPP
