//
// Created by tomer on 10/24/19.
//
/**
 * The core of the code is in the function find_attempt.
 * Abstract_index i is the index the i'th remainder will be stored, inside an unpacked list,
 * containing the remainders (one remainder in each index). In other words, suppose to represent the i'th
 * remainder stored inside Body.
 */

#ifndef CLION_CODE_BODY_H
#define CLION_CODE_BODY_H

#include <iostream>
#include <cassert>
#include <vector>
#include <algorithm>
#include <ctgmath>
#include "../../Global_functions/basic_functions.h"
#include "../../Global_functions/macros.h"
#include "../../bit_operations/bit_word_converter.hpp"


class Body {
    size_t capacity;
    const size_t fp_size;
    const size_t max_capacity;
    const size_t size;
    BODY_BLOCK_TYPE *B;

public:
    Body(size_t m, size_t f, size_t l);

//    ~Body();

    /**
     *
     * @param abstract_body_start_index: see class documentation.
     * @param abstract_body_end_index
     * @param remainder
     * @return
     */
    bool lookup(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder);

    void insert(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder);

    void remove(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder);

    /**
     * Used with choice of two technique. First, check if the element belong to body.
     * If it is, remove it and return True. Otherwise, return false.
     * @param abstract_body_start_index
     * @param abstract_body_end_index
     * @param remainder
     * @return
     */
    bool conditional_remove(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder);

    bool wrap_lookup(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder);

    void wrap_insert(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder);

    void wrap_remove(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder);

    //todo return value can be boolean (if PD is not adaptive).
    /**
     *
     * @param abstract_body_start_index: remainder first possible location,
     * assuming the body is an abstract list of remainders, instead of array of packed remainder.
     * @param abstract_body_end_index: the remainder last possible location.
     * @param remainder
     * @param p_B_index: the actual body.B index in which the remainder is, or where it should be inserted.
     * @param p_bit_index: the bit index inside body.B[*p_B_index] from which the remainder start.
     * (or should be inserted)
     * @return 1 - When given remainder was found. Pointers will point to it.
     *         2 - When given remainder was not found. Pointers will point to first slot (in the run or it end)
     *         with remainder bigger than the one that was given (or to the first slot in the next run).
     */
    int
    find_attempt(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder, size_t *p_B_index,
                 size_t *p_bit_index);

private:
    /**
     *
     * @param abstract_body_start_index
     * @param abstract_body_end_index
     * @param remainder
     * @param p_B_index
     * @param p_bit_index
     * @return 1 - When given remainder was found. Pointers will point to it.
     *         2 - When given remainder was not found. Pointers will point to first slot (in the run or it end)
     *         with remainder bigger than the one that was given (or to the first slot in the next run).
     */
    int
    find(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder, size_t *p_B_index,
         size_t *p_bit_index);


    int find_helper(bool did_find, size_t current_b_index, size_t bits_left, size_t *p_B_index, size_t *p_bit_index);


public:
    /**Function used for printing*/
    /**
     * Store each remainder in different slot inside the array a;
     * @param a = uint32_t[max_capacity].
     */
    void store_consecutive_remainders(uint32_t *a);

    void print_consecutive();

    void naive_print();



    /**Vector functions.*/

    /*

    void print_consecutive_with_vector();

    void naive_print_with_vector();

    bool lookup(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder);

    void insert(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder);

    void remove(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder);

    int
    vector_find_kth_interval_simple(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder, size_t *p_B_index,
                size_t *p_bit_index);

    BODY_BLOCK_TYPE read_FP_from_vector_by_index(size_t bit_start_index);

    void write_FP_to_vector_by_index(size_t index, FP_TYPE remainder);

    bool compare_remainder_and_vector(size_t bit_start_index, FP_TYPE remainder);

    void vector_push(size_t vector_bit_counter);
*/
    /**Validating functions.*/

//    void validate_find(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder);


    size_t get_fp_size() const;

    size_t get_size() const;

    uint32_t *get_b() const;

    friend ostream &operator<<(ostream &os, const Body &body);
};

static inline int find_helper_attempt(bool did_find, size_t current_b_index, size_t bits_left, size_t *p_B_index,
                                      size_t *p_bit_index) {
    *p_B_index = current_b_index;
    *p_bit_index = BODY_BLOCK_SIZE - bits_left;
    return 2 - did_find;
}

#endif //CLION_CODE_BODY_H

