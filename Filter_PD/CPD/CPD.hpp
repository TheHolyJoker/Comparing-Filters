//
// Created by tomer on 3/13/20.
//

/*
 * Notation
 */

#ifndef CLION_CODE_CPD_HPP
#define CLION_CODE_CPD_HPP

#include <ostream>
#include "../Global_functions/basic_functions.h"
#include "../Global_functions/cuckoo_and_counting_macros.hpp"
#include "../bit_operations/bit_op.h"
#include "../bit_operations/bit_word_converter.hpp"

#define CPD_DB_MODE0 (false)
#define CPD_V_MODE (true)
#define CPD_DB_MODE1 (true)

#define CPD_DB_MODE2 (CPD_DB_MODE0 || DB)

class CPD {
    CPD_TYPE *a;
    const uint_fast16_t fp_size, max_distinct_capacity, size, counter_size;
    const bool header_and_body_has_joined_slot, body_and_counters_has_joined_slot;

public:
    /**
     *
     * @param m range?
     * @param f max_distinct_capacity
     * @param l fp_size
     * @param counter_size number of bits each counter has.
     */
    CPD(size_t m, size_t f, size_t l, size_t counter_size);

//    virtual ~CPD();

    auto lookup(CPD_TYPE q, CPD_TYPE r) -> bool;

    /**
     *
     * @param q
     * @param r
     * @return 0 if not in the PD. Its multiplicity otherwise.
     */
    auto lookup_multi(CPD_TYPE q, CPD_TYPE r) -> size_t;

//    void insert(CPD_TYPE q, CPD_TYPE r);

    /**
     * Tries to insert an element.
     * First, checks if it is in the PD. If it is, tries to increase its counter. In case of an overflow, the element
     * *should be deleted* (not implemented yet).
     * If it is not in the PD, insert it to the PD.
     * If that
     * @param q
     * @param r
     * @return
     */
    auto insert(CPD_TYPE q, CPD_TYPE r) -> counter_status;

    /**
     * when an element is dropped from a higher level.
     * @param q
     * @param r
     * @param counter
     * @return
     */
    void insert_new_element_with_counter(CPD_TYPE q, CPD_TYPE r, CPD_TYPE counter);

//    void insert_new(CPD_TYPE r, size_t end_index);

    auto insert_inc_attempt(CPD_TYPE q, CPD_TYPE r) -> counter_status;

    void remove(CPD_TYPE q, CPD_TYPE r);

    void remove_old(CPD_TYPE q, CPD_TYPE r);

    /**
     * Removes the element only if it is in the PD.
     * @param quotient
     * @param remainder
     * @return true if the element was removed, false otherwise.
     */
    auto conditional_remove_old(CPD_TYPE q, CPD_TYPE r) -> bool;

    auto conditional_remove(CPD_TYPE q, CPD_TYPE r) -> counter_status;

    auto get_capacity() -> size_t;

    auto is_full() -> bool;

    auto is_empty() -> bool;


    auto has_unique_remainders() -> bool;


private:

    /**
     * Called when the inserted element is a member of the PD.
     * @param r
     * @param end_index
     * @param A_index
     * @param rel_bit_index
     * @return
     */
    auto insert_inc_helper(CPD_TYPE r, size_t end_index, size_t A_index, size_t rel_bit_index) -> counter_status;

    /**
     * Called when the inserted element is \NOT a member of the PD.
     * @param r
     * @param end_index
     * @param A_index
     * @param rel_bit_index
     * @return not a member;
     */
    auto insert_new_helper(CPD_TYPE r, size_t end_index, size_t A_index, size_t rel_bit_index) -> counter_status;

    auto insert_new_helper(CPD_TYPE r, size_t end_index, size_t A_index, size_t rel_bit_index,
                           CPD_TYPE counter) -> counter_status;

    auto insert_overflow_handler(CPD_TYPE r, size_t end_index, size_t A_index, size_t rel_bit_index) -> void;

    auto naive_conditional_remove(CPD_TYPE q, CPD_TYPE r) -> bool;

    auto get_last_a_index_containing_the_header() const -> size_t;

    auto get_last_a_index_containing_the_body() const -> size_t;

    auto get_first_index_containing_the_body() const -> size_t;

    auto get_first_index_containing_the_counters() const -> size_t;

    void print_as_consecutive_memory();

    auto read_counter(size_t counter_index) -> CPD_TYPE;

    void write_counter(size_t counter_index, CPD_TYPE value);

    auto header_lookup(CPD_TYPE q, size_t *start_index, size_t *end_index) -> bool;

    void header_insert(CPD_TYPE q, size_t *start_index, size_t *end_index);

    void header_push_wrapper(size_t end);

    void header_remove(CPD_TYPE q, size_t *start_index, size_t *end_index);

    void header_pull_wrapper(size_t end);

    void header_find(CPD_TYPE q, size_t *start, size_t *end);

    void header_push(size_t end_index);

    void header_pull(size_t end_index);

    static auto get_number_of_bits_in_a(size_t m, size_t f, size_t l) -> size_t;


    auto get_header_rel_bit_index_in_last_header_slot() -> size_t;

    auto get_body_rel_bit_index_in_last_body_slot() -> size_t;

    inline auto get_header_size_in_bits() -> size_t {
        return max_distinct_capacity << 1u;
    }

    inline auto total_bit_before_counter_begin() const -> size_t {
        return (2 + fp_size) * max_distinct_capacity;
    }

    inline auto get_body_last_index() -> size_t {
        assert(false);
        return my_ceil(total_bit_before_counter_begin(), CPD_TYPE_SIZE);
    }
/*

//    auto get_header_and_body_joined_slot_index() -> size_t;
//
//    auto get_body_and_counters_joined_slot_index() -> size_t;
*/

    /**
     * Used for validation.
     * @param r
     * @param unpacked_start_index
     * @param unpacked_end_index
     * @param p_array_index
     * @param p_bit_index
     * @return
     */
    auto body_find_wrapper(CPD_TYPE r, size_t unpacked_start_index, size_t unpacked_end_index, size_t *p_array_index,
                           size_t *p_bit_index) -> bool;


    /**
     * /todo validate?
     * @param r: remainder.
     * @param unpacked_start_index: remainder first possible location, assuming the body is an abstract list of
     * remainders, instead of array of packed remainder.
     * @param unpacked_end_index: the remainder last possible location.
     * @param p_array_index: the actual body.B index in which the remainder is, or where it should be inserted.
     * @param p_bit_index: the bit index inside body.B[*p_B_index] from which the remainder start.
     * (or should be inserted)
     * @return true - When given remainder was found. Pointers will point to it.
     *         false - When given remainder was not found. Pointers will point to first slot (in the run or it end)
     *         with remainder bigger than the one that was given (or to the first slot in the next run).
     */
    auto body_find(CPD_TYPE r, size_t unpacked_start_index, size_t unpacked_end_index, size_t *p_array_index,
                   size_t *p_bit_index) -> bool;


    /**
     * Denote the remainder at A[A_index][rel_bit_start_index:rel_bit_start_index + fp_size], by r. (python syntax where A is a binary string)
     * Returns r's unpacked index.
     * @param A_index
     * @param rel_bit_start_index
     * @return
     */
    auto translate_to_unpacked_index(size_t A_index, size_t rel_bit_start_index) -> size_t;

/*
//    auto
//    body_find_remainder_index(CPD_TYPE r, size_t unpacked_start_index, size_t unpacked_end_index, size_t *p_array_index,
//                              size_t *p_bit_index) -> size_t;
*/

    auto translate_counter_index_to_abs_bit_index(size_t counter_index) -> size_t;

    void body_insert(CPD_TYPE r, size_t unpacked_start_index, size_t unpacked_end_index);

    /**
     * Motivation: when push is preformed, the last slot of body, might contains part of the counters,
     * and the push could damage them. This function deals with it.
     * @param r
     * @param unpacked_start_index
     * @param unpacked_end_index
     */
    void body_push_wrapper(CPD_TYPE r, size_t A_index, size_t rel_bit_index);

    void body_push(CPD_TYPE r, size_t A_index, size_t rel_bit_index);

    void body_push_old(CPD_TYPE r, size_t B_index, size_t bit_index);

    void body_remove(CPD_TYPE r, size_t unpacked_start_index, size_t unpacked_end_index);

//    void body_remove_helper(size_t A_index, size_t rel_bit_index);

    auto body_conditional_remove(CPD_TYPE r, size_t unpacked_start_index, size_t unpacked_end_index) -> bool;

    inline void body_find_helper(size_t current_b_index, size_t bits_left, size_t *p_B_index, size_t *p_bit_index) {
        *p_B_index = current_b_index;
        *p_bit_index = BODY_BLOCK_SIZE - bits_left;
    }

    /**
     * See \body_push_wrapper.
     * @param B_index
     * @param bit_index
     */
    void body_pull_wrapper(size_t B_index, size_t bit_index);

    /**
     * I think! Moves all elements after the element that starts at "bit_index" to their predecessor position.
     * @param B_index
     * @param bit_index
     */
    void body_pull(size_t B_index, size_t bit_index);

    void counter_push(CPD_TYPE val, size_t bit_count_from_start);

    void counter_pull(size_t bit_count_from_start);

    void counter_pull_wrapper(size_t bit_count_from_start);

    auto increase_counter(size_t counter_index) -> counter_status;

    auto decrease_counter(size_t counter_index) -> counter_status;


public:
    /*For testing*/
//    auto get_a() const -> uint32_t *;

    auto get_a() const -> CPD_TYPE *;

    auto get_fp_size() const -> const uint_fast16_t;

    auto get_max_capacity() const -> const uint_fast16_t;

    auto get_size() const -> const uint_fast16_t;

    auto get_deal_with_joined_slot() const -> const bool;

    void print_as_array() const;

    void print_body_as_array() const;

    void print_body_as_consecutive_memory() const;

    void print_counters_as_array() const;

    void print_counters_as_consecutive_memory() const;


    friend auto operator<<(ostream &os, const CPD &cpd) -> ostream &;


//    friend auto operator<<(ostream &os, const cg_PD &pd) -> ostream &;

    static auto do_header_and_body_has_joined_slot(size_t m, size_t f, size_t l) -> bool {
        return bool((f * 2) % CPD_TYPE_SIZE);
    }

    static auto do_body_and_counters_has_joined_slot(size_t m, size_t f, size_t l, size_t counter_size) -> bool {
        size_t val = (f * counter_size) % CPD_TYPE_SIZE;
        bool res = (f * counter_size) % CPD_TYPE_SIZE;
        if (val > 0) {
            assert(res);
        } else {
            assert(!res);
        }
        return (f * counter_size) % CPD_TYPE_SIZE;
    }


};

static auto
get_a_size(size_t q_range, size_t max_distinct_element, size_t remainder_size, size_t counter_size) -> size_t;





/*
class CPD {
    CPD_TYPE *a;
    const uint_fast16_t fp_size, max_distinct_capacity, size;
    const bool header_and_body_has_joined_slot;

};
*/


#endif //CLION_CODE_CPD_HPP
